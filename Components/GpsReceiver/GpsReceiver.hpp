#ifndef PIPICUBE_GPS_RECEIVER_HPP
#define PIPICUBE_GPS_RECEIVER_HPP

#include <Components/GpsReceiver/GpsReceiverComponentAc.hpp>
#include <config/FpConfig.h>

namespace PipiCube {

static const uint32_t NMEA_LINE_BUF_SIZE = 100U;

class GpsReceiver final : public GpsReceiverComponentBase {
  public:
    explicit GpsReceiver(const char *compName);
    ~GpsReceiver() override;

    void init(const NATIVE_INT_TYPE instance = 0);

  private:
    // ── F' autocoded virtual methods ─────────────────────────────────────────

    void schedIn_handler(NATIVE_INT_TYPE portNum,
                         NATIVE_UINT_TYPE context) override;

    void GPS_RESET_cmdHandler(FwOpcodeType opCode,
                               U32          cmdSeq) override;

    void GPS_SET_ENABLED_cmdHandler(FwOpcodeType opCode,
                                     U32          cmdSeq,
                                     bool         enabled) override;

    // ── NMEA helpers ─────────────────────────────────────────────────────────

    // Try to process the fully assembled line in m_lineBuf[0..m_lineLen-1].
    void processLine(void);

    // Verify NMEA checksum. Returns true if valid.
    bool verifyChecksum(const char *line, uint32_t len) const;

    // Parse $GPGGA / $GNGGA sentence. Returns true on success.
    bool parseGGA(const char *line);

    // Convert ddmm.mmmm string + hemisphere char to millionths-of-degree I32.
    // Returns false if parsing fails.
    bool parseDegMin(const char *field, char hemi, I32 &out) const;

    // Parse a fixed-point decimal string × scale into I32.
    // e.g. parseFP("545.4", 100) → 54540
    bool parseFP(const char *s, uint32_t scale, I32 &out) const;

    // Parse unsigned integer from string.
    bool parseU8(const char *s, U8 &out) const;
    bool parseU16(const char *s, U16 &out) const;

    // ── State ─────────────────────────────────────────────────────────────────
    char     m_lineBuf[NMEA_LINE_BUF_SIZE];
    uint32_t m_lineLen;
    bool     m_enabled;
    U8       m_lastFixQuality;  // detect fix acquired / lost
    U32      m_sentenceCount;
};

} // namespace PipiCube

#endif // PIPICUBE_GPS_RECEIVER_HPP
