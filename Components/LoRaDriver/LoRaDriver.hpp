#ifndef PIPICUBE_LORA_DRIVER_HPP
#define PIPICUBE_LORA_DRIVER_HPP

#include <Components/LoRaDriver/LoRaDriverComponentAc.hpp>
#include <config/FpConfig.h>

namespace PipiCube {

static const uint32_t LORA_AT_BUF_SIZE    = 128U;
static const uint32_t LORA_PAYLOAD_MAX    = 80U;
static const uint32_t LORA_INIT_RETRIES   = 3U;
static const uint32_t LORA_AT_TIMEOUT_US  = 200000U;   // 200 ms

// Module state machine
enum class LoRaState : uint8_t {
    UNINIT  = 0,
    READY   = 1,
    ERROR   = 2
};

class LoRaDriver final : public LoRaDriverComponentBase {
  public:
    explicit LoRaDriver(const char *compName);
    ~LoRaDriver() override;

    void init(const NATIVE_INT_TYPE instance = 0);
    void preamble(void) override;   // runs init sequence after topology is set up

  private:
    // ── F' autocoded virtual methods ─────────────────────────────────────────

    void schedIn_handler(NATIVE_INT_TYPE portNum,
                         NATIVE_UINT_TYPE context) override;

    void dataIn_handler(NATIVE_INT_TYPE portNum,
                        Fw::Buffer &fwBuffer) override;

    void LORA_SEND_AT_cmdHandler(FwOpcodeType opCode,
                                  U32          cmdSeq,
                                  const Fw::CmdStringArg &cmd) override;

    void LORA_SET_FREQ_cmdHandler(FwOpcodeType opCode,
                                   U32          cmdSeq,
                                   U32          freqKHz) override;

    void LORA_SET_SF_cmdHandler(FwOpcodeType opCode,
                                 U32          cmdSeq,
                                 U8           sf) override;

    void LORA_SET_POWER_cmdHandler(FwOpcodeType opCode,
                                    U32          cmdSeq,
                                    U8           powerDbm) override;

    void LORA_RESET_cmdHandler(FwOpcodeType opCode,
                                U32          cmdSeq) override;

    // ── AT command helpers ────────────────────────────────────────────────────

    // Send an AT command string (adds \r\n). Returns true if +OK received.
    bool sendAT(const char *cmd, uint32_t timeout_us = LORA_AT_TIMEOUT_US);

    // Run the full RF init sequence. Returns true on success.
    bool initModule(void);

    // Read characters from UART0 ring buffer into m_atBuf until \r\n or buf full.
    // Returns the number of bytes read.
    uint32_t readATLine(uint32_t timeout_us);

    // Poll the ring buffer for a +RCV= line and parse it.
    void pollRx(void);

    // Parse "+RCV=<addr>,<len>,<data>,<rssi>,<snr>"
    bool parseRcv(const char *line, I16 &rssi, I16 &snr);

    // ── State ─────────────────────────────────────────────────────────────────
    LoRaState m_state;
    char      m_atBuf[LORA_AT_BUF_SIZE];
    uint32_t  m_atBufLen;
    U32       m_txCount;
    U32       m_txErrors;
    U32       m_rxCount;
};

} // namespace PipiCube

#endif // PIPICUBE_LORA_DRIVER_HPP
