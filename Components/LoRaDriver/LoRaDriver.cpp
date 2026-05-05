#include "LoRaDriver.hpp"
#include <hal/rp2040_hal.h>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/String.hpp>
#include <cstring>

namespace PipiCube {

// Compile-time format helpers
static const char *AT_CRLF = "\r\n";

// Convert U32 to decimal string. Returns pointer to end of string.
static char *u32toa(uint32_t val, char *buf, uint32_t buf_len) {
    if (buf_len < 2U) { buf[0] = '\0'; return buf; }
    if (val == 0U) { buf[0] = '0'; buf[1] = '\0'; return &buf[1]; }
    char tmp[11];
    uint32_t i = 0U;
    while (val > 0U && i < sizeof(tmp) - 1U) {
        tmp[i++] = static_cast<char>('0' + (val % 10U));
        val /= 10U;
    }
    // Reverse
    uint32_t out = 0U;
    for (uint32_t j = i; j > 0U && out < buf_len - 1U; j--) {
        buf[out++] = tmp[j - 1U];
    }
    buf[out] = '\0';
    return &buf[out];
}

LoRaDriver::LoRaDriver(const char *compName)
    : LoRaDriverComponentBase(compName),
      m_state(LoRaState::UNINIT),
      m_atBufLen(0U),
      m_txCount(0U),
      m_txErrors(0U),
      m_rxCount(0U)
{
    for (uint32_t i = 0U; i < LORA_AT_BUF_SIZE; i++) {
        m_atBuf[i] = '\0';
    }
}

LoRaDriver::~LoRaDriver() {}

void LoRaDriver::init(FwEnumStoreType instance) {
    LoRaDriverComponentBase::init(instance);
}

void LoRaDriver::preamble(void) {
    if (!initModule()) {
        m_state = LoRaState::ERROR;
        this->log_FATAL_InitFailed(static_cast<U8>(LORA_INIT_RETRIES));
        this->tlmWrite_ModuleState(static_cast<U8>(m_state));
    } else {
        m_state = LoRaState::READY;
        this->log_ACTIVITY_HI_ModuleReady();
        this->tlmWrite_ModuleState(static_cast<U8>(m_state));
    }
}

// ── initModule ────────────────────────────────────────────────────────────────
bool LoRaDriver::initModule(void) {
    // Step 1: Verify module alive (retry × LORA_INIT_RETRIES)
    bool alive = false;
    for (uint32_t attempt = 0U; attempt < LORA_INIT_RETRIES; attempt++) {
        if (sendAT("AT")) {
            alive = true;
            break;
        }
        hal_delay_ms(200U);
    }
    if (!alive) { return false; }

    // Step 2: Hardware reset (wait up to 2 s for +READY)
    if (!sendAT("AT+RESET", 2000000U)) { return false; }
    hal_delay_ms(500U);  // settle after reset

    // Step 3–7: Configure RF parameters
    char cmd[LORA_AT_BUF_SIZE];

    // Frequency (default 868 MHz EU)
    {
        uint32_t freq_hz = 868000000U;
        char freq_str[12];
        u32toa(freq_hz, freq_str, sizeof(freq_str));
        uint32_t cmd_len = 0U;
        const char *prefix = "AT+BAND=";
        while (*prefix && cmd_len < sizeof(cmd) - 2U) { cmd[cmd_len++] = *prefix++; }
        uint32_t i = 0U;
        while (freq_str[i] && cmd_len < sizeof(cmd) - 1U) { cmd[cmd_len++] = freq_str[i++]; }
        cmd[cmd_len] = '\0';
        if (!sendAT(cmd)) { return false; }
    }

    // SF=9, BW=125, CR=4/5, preamble=12
    if (!sendAT("AT+PARAMETER=9,7,1,12")) { return false; }

    // Node address
    if (!sendAT("AT+ADDRESS=1")) { return false; }

    // Network ID
    if (!sendAT("AT+NETWORKID=6")) { return false; }

    // Output power 14 dBm
    if (!sendAT("AT+CRFOP=14")) { return false; }

    return true;
}

// ── sendAT ────────────────────────────────────────────────────────────────────
bool LoRaDriver::sendAT(const char *cmd, uint32_t timeout_us) {
    // Send command + \r\n
    size_t cmd_len = 0U;
    while (cmd[cmd_len] != '\0' && cmd_len < LORA_AT_BUF_SIZE - 2U) { cmd_len++; }

    int32_t rc = hal_uart_write(HAL_UART0,
                                 reinterpret_cast<const uint8_t *>(cmd),
                                 cmd_len, 5000U);
    if (rc != HAL_OK) { return false; }
    rc = hal_uart_write(HAL_UART0,
                         reinterpret_cast<const uint8_t *>(AT_CRLF),
                         2U, 5000U);
    if (rc != HAL_OK) { return false; }

    // Wait for response line
    uint32_t resp_len = readATLine(timeout_us);
    if (resp_len == 0U) { return false; }

    // +OK or +RESET or +READY all indicate success for init steps
    bool ok = (m_atBuf[0] == '+') &&
              ((m_atBuf[1] == 'O') ||   // +OK
               (m_atBuf[1] == 'R'));    // +RESET, +READY
    return ok;
}

// ── readATLine ───────────────────────────────────────────────────────────────
uint32_t LoRaDriver::readATLine(uint32_t timeout_us) {
    uint64_t deadline = hal_time_us() + static_cast<uint64_t>(timeout_us);
    m_atBufLen = 0U;

    while (hal_time_us() < deadline) {
        uint8_t byte = 0U;
        if (hal_uart_getc(HAL_UART0, &byte) == HAL_OK) {
            if (byte == '\n') {
                m_atBuf[m_atBufLen] = '\0';
                return m_atBufLen;
            } else if (byte != '\r') {
                if (m_atBufLen < LORA_AT_BUF_SIZE - 1U) {
                    m_atBuf[m_atBufLen++] = static_cast<char>(byte);
                } else {
                    this->log_WARNING_HI_AtBufOverrun();
                    m_atBufLen = 0U;
                    return 0U;
                }
            }
        }
    }
    return 0U;   // timeout
}

// ── schedIn_handler ────────────────────────────────────────────────────────────
void LoRaDriver::schedIn_handler(FwIndexType portNum,
                                  U32 context) {
    (void)portNum;
    (void)context;
    if (m_state != LoRaState::READY) { return; }
    pollRx();
}

// ── pollRx ────────────────────────────────────────────────────────────────────
void LoRaDriver::pollRx(void) {
    // Drain ring buffer looking for +RCV= lines
    uint8_t byte = 0U;
    uint32_t drain_limit = 256U;
    uint32_t drain_count = 0U;

    while (drain_count < drain_limit &&
           hal_uart_getc(HAL_UART0, &byte) == HAL_OK) {
        drain_count++;

        if (byte == '\n') {
            m_atBuf[m_atBufLen] = '\0';
            if (m_atBufLen >= 5U &&
                m_atBuf[0] == '+' && m_atBuf[1] == 'R' &&
                m_atBuf[2] == 'C' && m_atBuf[3] == 'V' &&
                m_atBuf[4] == '=') {
                I16 rssi = 0, snr = 0;
                if (parseRcv(m_atBuf, rssi, snr)) {
                    m_rxCount++;
                    this->tlmWrite_LastRxRssi_dBm(rssi);
                    this->tlmWrite_LastRxSnr_x10(snr);
                    this->tlmWrite_RxPacketCount(m_rxCount);
                    this->log_ACTIVITY_LO_PacketReceived(rssi, snr);
                }
            }
            m_atBufLen = 0U;
        } else if (byte != '\r') {
            if (m_atBufLen < LORA_AT_BUF_SIZE - 1U) {
                m_atBuf[m_atBufLen++] = static_cast<char>(byte);
            } else {
                this->log_WARNING_HI_AtBufOverrun();
                m_atBufLen = 0U;
            }
        }
    }
}

// ── parseRcv ─────────────────────────────────────────────────────────────────
// Parse "+RCV=<addr>,<len>,<data>,<rssi>,<snr>"
bool LoRaDriver::parseRcv(const char *line, I16 &rssi, I16 &snr) {
    // Skip "+RCV="
    const char *p = line + 5U;

    // Skip addr field (up to comma)
    uint32_t skip_limit = 8U;
    while (*p != '\0' && *p != ',' && skip_limit-- > 0U) { p++; }
    if (*p != ',') { return false; }
    p++;

    // Skip len field
    skip_limit = 4U;
    while (*p != '\0' && *p != ',' && skip_limit-- > 0U) { p++; }
    if (*p != ',') { return false; }
    p++;

    // Skip data field
    skip_limit = 100U;
    while (*p != '\0' && *p != ',' && skip_limit-- > 0U) { p++; }
    if (*p != ',') { return false; }
    p++;

    // Parse RSSI (signed)
    bool rssi_neg = (*p == '-');
    if (rssi_neg) { p++; }
    int32_t rssi_val = 0;
    uint32_t digit_limit = 6U;
    while (*p >= '0' && *p <= '9' && digit_limit-- > 0U) {
        rssi_val = rssi_val * 10 + (*p - '0');
        p++;
    }
    if (rssi_neg) { rssi_val = -rssi_val; }
    if (*p != ',') { return false; }
    p++;

    // Parse SNR (signed, may be decimal — store × 10)
    bool snr_neg = (*p == '-');
    if (snr_neg) { p++; }
    int32_t snr_int  = 0;
    int32_t snr_frac = 0;
    digit_limit = 4U;
    while (*p >= '0' && *p <= '9' && digit_limit-- > 0U) {
        snr_int = snr_int * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        if (*p >= '0' && *p <= '9') {
            snr_frac = *p - '0';
            p++;
        }
    }
    int32_t snr_val = snr_int * 10 + snr_frac;
    if (snr_neg) { snr_val = -snr_val; }

    rssi = static_cast<I16>(rssi_val);
    snr  = static_cast<I16>(snr_val);
    return true;
}

// ── dataIn_handler ────────────────────────────────────────────────────────────
void LoRaDriver::dataIn_handler(FwIndexType portNum,
                                 Fw::Buffer &fwBuffer) {
    (void)portNum;
    if (m_state != LoRaState::READY) { return; }

    uint32_t payload_len = fwBuffer.getSize();
    if (payload_len > LORA_PAYLOAD_MAX) {
        this->log_WARNING_HI_PayloadTooLarge(payload_len);
        return;
    }

    // Build AT+SEND=0,<len>,<data>\r\n
    char cmd[LORA_AT_BUF_SIZE];
    uint32_t idx = 0U;

    const char *prefix = "AT+SEND=0,";
    while (*prefix && idx < sizeof(cmd) - 1U) { cmd[idx++] = *prefix++; }

    char len_str[6];
    u32toa(payload_len, len_str, sizeof(len_str));
    uint32_t i = 0U;
    while (len_str[i] && idx < sizeof(cmd) - 1U) { cmd[idx++] = len_str[i++]; }

    if (idx < sizeof(cmd) - 1U) { cmd[idx++] = ','; }

    const uint8_t *data = fwBuffer.getData();
    for (uint32_t d = 0U; d < payload_len && idx < sizeof(cmd) - 1U; d++) {
        cmd[idx++] = static_cast<char>(data[d]);
    }
    cmd[idx] = '\0';

    // Transmit
    int32_t rc = hal_uart_write(HAL_UART0,
                                 reinterpret_cast<const uint8_t *>(cmd),
                                 idx, 10000U);
    if (rc != HAL_OK) {
        m_txErrors++;
        this->tlmWrite_TxErrorCount(m_txErrors);
        return;
    }
    rc = hal_uart_write(HAL_UART0,
                         reinterpret_cast<const uint8_t *>(AT_CRLF),
                         2U, 5000U);
    if (rc != HAL_OK) {
        m_txErrors++;
        this->tlmWrite_TxErrorCount(m_txErrors);
        return;
    }

    // Read response with a 1-second timeout
    uint32_t resp_len = readATLine(1000000U);
    if (resp_len == 0U || m_atBuf[0] == '+' && m_atBuf[1] == 'E') {
        // +ERR=<n>
        U8 err_code = 0U;
        if (resp_len >= 6U) {
            err_code = static_cast<U8>(m_atBuf[5] - '0');
        }
        this->log_WARNING_HI_TxError(err_code);
        m_txErrors++;
        this->tlmWrite_TxErrorCount(m_txErrors);
    } else {
        m_txCount++;
        this->tlmWrite_TxPacketCount(m_txCount);
    }
}

// ── Command handlers ──────────────────────────────────────────────────────────

void LoRaDriver::LORA_SEND_AT_cmdHandler(const FwOpcodeType opCode,
                                          const U32          cmdSeq,
                                          const Fw::CmdStringArg &cmd) {
    bool ok = sendAT(cmd.toChar());
    this->cmdResponse_out(0, opCode, cmdSeq,
                          ok ? Fw::CmdResponse::OK
                             : Fw::CmdResponse::EXECUTION_ERROR);
}

void LoRaDriver::LORA_SET_FREQ_cmdHandler(const FwOpcodeType opCode,
                                           const U32          cmdSeq,
                                           const U32          freqKHz) {
    char cmd[LORA_AT_BUF_SIZE];
    uint32_t idx = 0U;
    const char *prefix = "AT+BAND=";
    while (*prefix && idx < sizeof(cmd) - 1U) { cmd[idx++] = *prefix++; }
    char val[12];
    u32toa(freqKHz * 1000U, val, sizeof(val));
    uint32_t i = 0U;
    while (val[i] && idx < sizeof(cmd) - 1U) { cmd[idx++] = val[i++]; }
    cmd[idx] = '\0';

    bool ok = sendAT(cmd);
    this->cmdResponse_out(0, opCode, cmdSeq,
                          ok ? Fw::CmdResponse::OK
                             : Fw::CmdResponse::EXECUTION_ERROR);
}

void LoRaDriver::LORA_SET_SF_cmdHandler(const FwOpcodeType opCode,
                                         const U32          cmdSeq,
                                         const U8           sf) {
    if (sf < 7U || sf > 12U) {
        this->cmdResponse_out(0, opCode, cmdSeq,
                              Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    // AT+PARAMETER=<sf>,7,1,12  (BW=125, CR=4/5, preamble=12)
    char cmd[LORA_AT_BUF_SIZE];
    uint32_t idx = 0U;
    const char *prefix = "AT+PARAMETER=";
    while (*prefix && idx < sizeof(cmd) - 1U) { cmd[idx++] = *prefix++; }
    cmd[idx++] = static_cast<char>('0' + sf);
    const char *suffix = ",7,1,12";
    while (*suffix && idx < sizeof(cmd) - 1U) { cmd[idx++] = *suffix++; }
    cmd[idx] = '\0';

    bool ok = sendAT(cmd);
    this->cmdResponse_out(0, opCode, cmdSeq,
                          ok ? Fw::CmdResponse::OK
                             : Fw::CmdResponse::EXECUTION_ERROR);
}

void LoRaDriver::LORA_SET_POWER_cmdHandler(const FwOpcodeType opCode,
                                            const U32          cmdSeq,
                                            const U8           powerDbm) {
    if (powerDbm > 22U) {
        this->cmdResponse_out(0, opCode, cmdSeq,
                              Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    char cmd[LORA_AT_BUF_SIZE];
    uint32_t idx = 0U;
    const char *prefix = "AT+CRFOP=";
    while (*prefix && idx < sizeof(cmd) - 1U) { cmd[idx++] = *prefix++; }
    char val[4];
    u32toa(static_cast<uint32_t>(powerDbm), val, sizeof(val));
    uint32_t i = 0U;
    while (val[i] && idx < sizeof(cmd) - 1U) { cmd[idx++] = val[i++]; }
    cmd[idx] = '\0';

    bool ok = sendAT(cmd);
    this->cmdResponse_out(0, opCode, cmdSeq,
                          ok ? Fw::CmdResponse::OK
                             : Fw::CmdResponse::EXECUTION_ERROR);
}

void LoRaDriver::LORA_RESET_cmdHandler(const FwOpcodeType opCode,
                                        const U32          cmdSeq) {
    bool ok = initModule();
    if (ok) {
        m_state = LoRaState::READY;
        this->log_ACTIVITY_HI_ModuleReady();
    } else {
        m_state = LoRaState::ERROR;
        this->log_FATAL_InitFailed(static_cast<U8>(LORA_INIT_RETRIES));
    }
    this->tlmWrite_ModuleState(static_cast<U8>(m_state));
    this->cmdResponse_out(0, opCode, cmdSeq,
                          ok ? Fw::CmdResponse::OK
                             : Fw::CmdResponse::EXECUTION_ERROR);
}

} // namespace PipiCube
