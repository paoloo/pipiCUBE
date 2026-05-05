module PipiCube {

  @ Active component that polls the IP5306 battery management IC over I2C1.
  @ Scheduled at 1 Hz. Reports charge state, level, and output status as
  @ telemetry; emits events on threshold crossings and charge-state changes.
  active component BatteryMonitor {

    # ── Standard F' infrastructure ports ──────────────────────────────────────
    command recv port cmdIn
    command reg  port cmdRegOut
    command resp port cmdResponseOut

    event      port logOut
    text event port logTextOut
    time get   port timeCaller
    telemetry  port tlmOut

    # ── Scheduling (1 Hz rate group) ─────────────────────────────────────────
    sync input port schedIn: Svc.Sched

    # ── Telemetry channels ────────────────────────────────────────────────────

    @ Battery charge level in percent (0–100). Maps IP5306 4-LED field:
    @ [0-25%]→12, [26-50%]→37, [51-75%]→62, [76-99%]→88, full→100.
    telemetry BatteryPercent: U8 id 0x100 update always

    @ IP5306 charge state (0=not charging, 1=pre-charge, 2=CC, 3=CV, 4=done)
    telemetry ChargeState: U8 id 0x101 update always

    @ Boost/output enable bit from IP5306 READ3
    telemetry OutputEnabled: bool id 0x102 update on change

    @ Cumulative I2C read error count since boot
    telemetry I2cErrors: U32 id 0x103 update on change

    # ── Events ────────────────────────────────────────────────────────────────

    @ I2C transaction to IP5306 failed
    event I2cReadError(
      regAddr:  U8   @< register address attempted
      errCode: I32  @< HAL error code
    ) severity warning low id 0x100 \
      format "IP5306 I2C read reg=0x{x} err={}"

    @ Battery percent dropped to or below the configurable low threshold
    event BatteryLow(
      percent: U8
    ) severity warning high id 0x101 \
      format "Battery low: {}%"

    @ Battery percent dropped to or below the hard-coded critical threshold (5%)
    @ — initiates safe-mode telemetry shutdown via LoRa
    event BatteryCritical(
      percent: U8
    ) severity fatal id 0x102 \
      format "Battery CRITICAL {}% — entering safe mode"

    @ IP5306 charge state register changed value
    event ChargeStateChanged(
      newState: U8
    ) severity activity low id 0x103 \
      format "Charge state → {}"

    @ Battery reported fully charged (CHARGE_FULL bit set in IP5306 READ0)
    event BatteryFull severity activity high id 0x104 \
      format "Battery fully charged"

    # ── Commands ──────────────────────────────────────────────────────────────

    @ Trigger an immediate out-of-schedule battery poll
    async command BATTERY_POLL opcode 0x00

    @ Update the low-battery warning threshold (0–100 %).
    @ Values outside [1, 99] are rejected with VALIDATION_ERROR.
    async command BATTERY_SET_LOW_THRESHOLD(
      threshold: U8
    ) opcode 0x01

  }

}
