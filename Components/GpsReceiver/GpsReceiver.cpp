#include "GpsReceiver.hpp"
#include <hal/rp2040_hal.h>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/String.hpp>

namespace PipiCube {

GpsReceiver::GpsReceiver(const char *compName)
    : GpsReceiverComponentBase(compName),
      m_lineLen(0U),
      m_enabled(true),
      m_lastFixQuality(0U),
      m_sentenceCount(0U)
{
    // Zero-init the static line buffer
    for (uint32_t i = 0U; i < NMEA_LINE_BUF_SIZE; i++) {
        m_lineBuf[i] = '\0';
    }
}

GpsReceiver::~GpsReceiver() {}

void GpsReceiver::init(FwEnumStoreType instance) {
    GpsReceiverComponentBase::init(instance);
}

// ── schedIn_handler ────────────────────────────────────────────────────────────
void GpsReceiver::schedIn_handler(FwIndexType portNum,
                                   U32 context) {
    (void)portNum;
    (void)context;

    if (!m_enabled) { return; }

    // Drain the UART1 ring buffer. Hard upper bound = ring buf size (256).
    uint8_t byte = 0U;
    uint32_t drain_limit = 256U;
    uint32_t drain_count = 0U;

    while (drain_count < drain_limit &&
           hal_uart_getc(HAL_UART1, &byte) == HAL_OK) {
        drain_count++;

        if (byte == '\n') {
            if (m_lineLen > 0U) {
                this->processLine();
            }
            m_lineLen = 0U;
        } else if (byte != '\r') {
            if (m_lineLen < NMEA_LINE_BUF_SIZE - 1U) {
                m_lineBuf[m_lineLen] = static_cast<char>(byte);
                m_lineLen++;
            } else {
                // Buffer full without newline — overrun
                this->log_WARNING_HI_UartOverrun(m_lineLen);
                m_lineLen = 0U;
            }
        }
    }
}

// ── processLine ───────────────────────────────────────────────────────────────
void GpsReceiver::processLine(void) {
    m_lineBuf[m_lineLen] = '\0';

    // Only process GGA sentences ($GPGGA or $GNGGA for multi-constellation)
    bool is_gga = (m_lineLen >= 6U) &&
                  (m_lineBuf[0] == '$') &&
                  (m_lineBuf[3] == 'G') &&
                  (m_lineBuf[4] == 'G') &&
                  (m_lineBuf[5] == 'A');
    if (!is_gga) { return; }

    if (!verifyChecksum(m_lineBuf, m_lineLen)) {
        Fw::String sentenceStr(m_lineBuf);
        this->log_WARNING_LO_NmeaChecksumError(sentenceStr);
        return;
    }

    if (parseGGA(m_lineBuf)) {
        m_sentenceCount++;
        this->tlmWrite_SentenceCount(m_sentenceCount);
    }
}

// ── verifyChecksum ────────────────────────────────────────────────────────────
bool GpsReceiver::verifyChecksum(const char *line, uint32_t len) const {
    // Find '*'
    uint32_t star_pos = 0U;
    bool found = false;
    for (uint32_t i = 0U; i < len; i++) {
        if (line[i] == '*') {
            star_pos = i;
            found = true;
            break;
        }
    }
    if (!found || (star_pos + 2U) >= len) {
        return false;
    }

    // XOR bytes between '$' (exclusive) and '*' (exclusive)
    uint8_t calc = 0U;
    for (uint32_t i = 1U; i < star_pos; i++) {
        calc ^= static_cast<uint8_t>(line[i]);
    }

    // Parse two hex digits after '*'
    auto hexDigit = [](char c) -> int32_t {
        if (c >= '0' && c <= '9') { return c - '0'; }
        if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
        if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
        return -1;
    };
    int32_t hi = hexDigit(line[star_pos + 1U]);
    int32_t lo = hexDigit(line[star_pos + 2U]);
    if (hi < 0 || lo < 0) { return false; }

    uint8_t expected = static_cast<uint8_t>((hi << 4) | lo);
    return calc == expected;
}

// ── parseGGA ──────────────────────────────────────────────────────────────────
// Tokenises the GGA sentence and extracts fields into telemetry.
bool GpsReceiver::parseGGA(const char *line) {
    // Tokenise by comma into a pointer array (no malloc)
    static const uint32_t MAX_FIELDS = 16U;
    const char *fields[MAX_FIELDS];
    uint32_t    field_count = 0U;
    uint32_t    line_len    = 0U;

    // Count length (already NUL-terminated)
    while (line[line_len] != '\0' && line_len < NMEA_LINE_BUF_SIZE) {
        line_len++;
    }

    // Work on a local copy to insert NULs as delimiters
    static char scratch[NMEA_LINE_BUF_SIZE];
    uint32_t copy_len = (line_len < NMEA_LINE_BUF_SIZE - 1U)
                        ? line_len : NMEA_LINE_BUF_SIZE - 1U;
    for (uint32_t i = 0U; i < copy_len; i++) {
        scratch[i] = line[i];
    }
    scratch[copy_len] = '\0';

    fields[field_count++] = scratch;
    for (uint32_t i = 0U; i < copy_len && field_count < MAX_FIELDS; i++) {
        if (scratch[i] == ',') {
            scratch[i]            = '\0';
            fields[field_count++] = &scratch[i + 1U];
        }
    }

    // Need at least 10 fields for altitude
    if (field_count < 10U) {
        this->log_WARNING_LO_ParseError(0U);
        return false;
    }

    // fields[0] = "$GPGGA"
    // fields[1] = hhmmss.ss
    // fields[2] = ddmm.mmmm  (latitude)
    // fields[3] = N/S
    // fields[4] = dddmm.mmmm (longitude)
    // fields[5] = E/W
    // fields[6] = fix quality
    // fields[7] = satellites
    // fields[8] = HDOP
    // fields[9] = altitude MSL
    // fields[10]= M

    // UTC time: hhmmss.ss
    U8 hh = 0U, mm = 0U, ss = 0U;
    const char *t = fields[1];
    uint32_t tlen = 0U;
    while (t[tlen] != '\0' && tlen < 16U) { tlen++; }
    if (tlen >= 6U) {
        hh = static_cast<U8>((t[0]-'0')*10 + (t[1]-'0'));
        mm = static_cast<U8>((t[2]-'0')*10 + (t[3]-'0'));
        ss = static_cast<U8>((t[4]-'0')*10 + (t[5]-'0'));
    }
    U32 utc_hms = (static_cast<U32>(hh) << 16U) |
                  (static_cast<U32>(mm) <<  8U) |
                   static_cast<U32>(ss);
    this->tlmWrite_UtcTime_hms(utc_hms);

    // Latitude
    I32 lat = 0;
    if (!parseDegMin(fields[2], fields[3][0], lat)) {
        this->log_WARNING_LO_ParseError(2U);
        return false;
    }

    // Longitude
    I32 lon = 0;
    if (!parseDegMin(fields[4], fields[5][0], lon)) {
        this->log_WARNING_LO_ParseError(4U);
        return false;
    }

    // Fix quality
    U8 fix_quality = 0U;
    if (!parseU8(fields[6], fix_quality)) {
        this->log_WARNING_LO_ParseError(6U);
        return false;
    }

    // Satellites
    U8 sats = 0U;
    if (!parseU8(fields[7], sats)) {
        this->log_WARNING_LO_ParseError(7U);
        return false;
    }

    // HDOP × 100
    I32 hdop_i32 = 0;
    if (!parseFP(fields[8], 100U, hdop_i32)) {
        this->log_WARNING_LO_ParseError(8U);
        return false;
    }
    U16 hdop_x100 = (hdop_i32 >= 0 && hdop_i32 <= 65535)
                    ? static_cast<U16>(hdop_i32) : 9999U;

    // Altitude cm (field in metres, scale ×100)
    I32 alt_cm = 0;
    if (!parseFP(fields[9], 100U, alt_cm)) {
        this->log_WARNING_LO_ParseError(9U);
        return false;
    }

    // Publish telemetry
    this->tlmWrite_FixQuality(fix_quality);
    this->tlmWrite_SatCount(sats);
    this->tlmWrite_Latitude_1e6deg(lat);
    this->tlmWrite_Longitude_1e6deg(lon);
    this->tlmWrite_Altitude_cm(alt_cm);
    this->tlmWrite_HDOP_x100(hdop_x100);

    // Fix acquired / lost events
    if (fix_quality > 0U && m_lastFixQuality == 0U) {
        this->log_ACTIVITY_HI_FixAcquired(sats);
    } else if (fix_quality == 0U && m_lastFixQuality > 0U) {
        this->log_WARNING_LO_FixLost();
    }
    m_lastFixQuality = fix_quality;

    return true;
}

// ── parseDegMin ───────────────────────────────────────────────────────────────
// Convert NMEA ddmm.mmmm + hemisphere to millionths-of-degree I32.
bool GpsReceiver::parseDegMin(const char *field, char hemi, I32 &out) const {
    if (field == nullptr || field[0] == '\0') {
        out = 0;
        return false;
    }

    // Find the decimal point to determine where degrees end
    uint32_t dot_pos = 0U;
    bool has_dot = false;
    for (uint32_t i = 0U; i < 16U && field[i] != '\0'; i++) {
        if (field[i] == '.') { dot_pos = i; has_dot = true; break; }
    }
    if (!has_dot || dot_pos < 3U) { out = 0; return false; }

    // Degrees are all but the last 2 digits before the decimal
    uint32_t deg_end = dot_pos - 2U;  // e.g. "4807.038" → dot_pos=4, deg_end=2

    // Parse degrees
    int32_t degrees = 0;
    for (uint32_t i = 0U; i < deg_end; i++) {
        if (field[i] < '0' || field[i] > '9') { out = 0; return false; }
        degrees = degrees * 10 + (field[i] - '0');
    }

    // Parse minutes as integer and fractional parts
    int32_t min_int = 0;
    for (uint32_t i = deg_end; i < dot_pos; i++) {
        if (field[i] < '0' || field[i] > '9') { out = 0; return false; }
        min_int = min_int * 10 + (field[i] - '0');
    }

    // Parse up to 4 fractional digits of minutes
    int32_t min_frac = 0;
    uint32_t frac_digits = 0U;
    for (uint32_t i = dot_pos + 1U;
         frac_digits < 4U && field[i] != '\0';
         i++, frac_digits++) {
        if (field[i] < '0' || field[i] > '9') break;
        min_frac = min_frac * 10 + (field[i] - '0');
    }
    // Pad to 4 fractional digits
    for (; frac_digits < 4U; frac_digits++) { min_frac *= 10; }

    // minutes in millionths: (min_int + min_frac/10000) / 60 * 1e6
    // = (min_int * 10000 + min_frac) * 1000000 / 60 / 10000
    // = (min_int * 10000 + min_frac) * 100 / 60
    int32_t total_min_x10000 = min_int * 10000 + min_frac;
    int32_t min_1e6 = (total_min_x10000 * 100) / 60;

    int32_t result = degrees * 1000000 + min_1e6;
    if (hemi == 'S' || hemi == 'W') {
        result = -result;
    }
    out = result;
    return true;
}

// ── parseFP ───────────────────────────────────────────────────────────────────
bool GpsReceiver::parseFP(const char *s, uint32_t scale, I32 &out) const {
    if (s == nullptr || s[0] == '\0') { out = 0; return false; }

    bool negative = (s[0] == '-');
    uint32_t idx = negative ? 1U : 0U;

    int32_t integer_part = 0;
    while (s[idx] != '\0' && s[idx] != '.') {
        if (s[idx] < '0' || s[idx] > '9') { out = 0; return false; }
        integer_part = integer_part * 10 + (s[idx] - '0');
        idx++;
    }

    int32_t frac_part = 0;
    uint32_t frac_scale = scale;
    if (s[idx] == '.') {
        idx++;
        uint32_t fs = scale;
        while (s[idx] != '\0' && fs > 1U) {
            if (s[idx] < '0' || s[idx] > '9') break;
            frac_part = frac_part * 10 + (s[idx] - '0');
            idx++;
            fs /= 10U;
        }
        frac_scale = scale / frac_scale * fs;  // remaining scale
        // Align frac_part to scale
        while (frac_scale > 1U) { frac_part *= 10; frac_scale /= 10U; }
    }

    int32_t result = integer_part * static_cast<int32_t>(scale) + frac_part;
    out = negative ? -result : result;
    return true;
}

bool GpsReceiver::parseU8(const char *s, U8 &out) const {
    if (s == nullptr || s[0] == '\0') { out = 0U; return true; }
    uint32_t val = 0U;
    for (uint32_t i = 0U; s[i] != '\0' && i < 4U; i++) {
        if (s[i] < '0' || s[i] > '9') { out = 0U; return false; }
        val = val * 10U + static_cast<uint32_t>(s[i] - '0');
    }
    if (val > 255U) { out = 0U; return false; }
    out = static_cast<U8>(val);
    return true;
}

bool GpsReceiver::parseU16(const char *s, U16 &out) const {
    if (s == nullptr || s[0] == '\0') { out = 0U; return true; }
    uint32_t val = 0U;
    for (uint32_t i = 0U; s[i] != '\0' && i < 6U; i++) {
        if (s[i] < '0' || s[i] > '9') { out = 0U; return false; }
        val = val * 10U + static_cast<uint32_t>(s[i] - '0');
    }
    if (val > 65535U) { out = 0U; return false; }
    out = static_cast<U16>(val);
    return true;
}

// ── Command handlers ──────────────────────────────────────────────────────────

void GpsReceiver::GPS_RESET_cmdHandler(const FwOpcodeType opCode,
                                        const U32          cmdSeq) {
    hal_uart_rx_flush(HAL_UART1);
    m_lineLen       = 0U;
    m_lastFixQuality = 0U;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void GpsReceiver::GPS_SET_ENABLED_cmdHandler(const FwOpcodeType opCode,
                                               const U32          cmdSeq,
                                               const bool         enabled) {
    m_enabled = enabled;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

} // namespace PipiCube
