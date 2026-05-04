module PipiCube {

  @ Active component that reads NMEA GGA sentences from the NEO-6M V2 GPS
  @ module on UART1 (GP4=TX, GP5=RX, 9600-8-N-1).
  @ Interrupt-driven ring buffer is drained at 1 Hz; all parsing is in-task.
  active component GpsReceiver {

    # ── Standard F' infrastructure ports ──────────────────────────────────────
    command recv port cmdIn
    command reg  port cmdRegOut
    command resp port cmdResponseOut

    event      port logOut
    text event port logTextOut
    time get   port timeCaller
    telemetry  port tlmOut

    # ── Scheduling (1 Hz) ────────────────────────────────────────────────────
    sync input port schedIn: Svc.Sched

    # ── Telemetry channels ────────────────────────────────────────────────────

    @ GGA fix quality: 0=invalid, 1=GPS fix, 2=DGPS fix
    telemetry FixQuality: U8 id 0x200 update always

    @ Satellites used in current fix
    telemetry SatCount: U8 id 0x201 update always

    @ Latitude in millionths of a degree (signed). Negative = South.
    @ e.g. 38°7.423'N → +38123717
    telemetry Latitude_1e6deg: I32 id 0x202 update always

    @ Longitude in millionths of a degree (signed). Negative = West.
    telemetry Longitude_1e6deg: I32 id 0x203 update always

    @ Altitude above mean sea level in centimetres (signed)
    telemetry Altitude_cm: I32 id 0x204 update always

    @ HDOP × 100 (e.g. HDOP 1.2 → 120)
    telemetry HDOP_x100: U16 id 0x205 update always

    @ UTC time packed as (hh<<16)|(mm<<8)|ss
    telemetry UtcTime_hms: U32 id 0x206 update always

    @ Total NMEA sentences processed since boot
    telemetry SentenceCount: U32 id 0x207 update on change

    # ── Events ────────────────────────────────────────────────────────────────

    @ NMEA XOR checksum did not match the two-hex-digit suffix
    event NmeaChecksumError(
      sentence: string size 32
    ) severity warning low id 0x200 \
      format "NMEA checksum error: {}"

    @ Fix quality transitioned from 0 (invalid) to ≥ 1 (valid)
    event FixAcquired(
      satellites: U8
    ) severity activity high id 0x201 \
      format "GPS fix acquired, sats={}"

    @ Fix quality transitioned to 0 (invalid)
    event FixLost severity warning low id 0x202 \
      format "GPS fix lost"

    @ NMEA line buffer overran (line > 100 bytes without a newline)
    event UartOverrun(
      droppedBytes: U32
    ) severity warning high id 0x203 \
      format "GPS UART line overrun, dropped {} bytes"

    @ GGA field failed range or format check
    event ParseError(
      fieldIndex: U8
    ) severity warning low id 0x204 \
      format "GGA parse error at field {}"

    # ── Commands ──────────────────────────────────────────────────────────────

    @ Flush the UART1 ring buffer and reset the line assembler
    async command GPS_RESET opcode 0x10

    @ Enable or disable GPS telemetry output (default: enabled)
    async command GPS_SET_ENABLED(
      enabled: bool
    ) opcode 0x11

  }

}
