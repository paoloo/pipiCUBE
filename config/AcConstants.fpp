# Project-wide F Prime constants.
# Referenced by topology FPP and component FPP files.

module PipiCube {

  # ── Queue depths ────────────────────────────────────────────────────────────
  constant BATTERY_MONITOR_QUEUE_DEPTH = 10
  constant GPS_RECEIVER_QUEUE_DEPTH    = 10
  constant LORA_DRIVER_QUEUE_DEPTH     = 20

  # ── Stack sizes (bytes) ─────────────────────────────────────────────────────
  constant BATTERY_MONITOR_STACK_SIZE  = 2048
  constant GPS_RECEIVER_STACK_SIZE     = 2048
  constant LORA_DRIVER_STACK_SIZE      = 4096

  # ── Thread priorities (higher = more urgent) ─────────────────────────────────
  constant BATTERY_MONITOR_PRIORITY    = 80
  constant GPS_RECEIVER_PRIORITY       = 75
  constant LORA_DRIVER_PRIORITY        = 70

  # ── HAL buffer sizes ────────────────────────────────────────────────────────
  constant HAL_UART_RING_BUF_SIZE      = 256
  constant NMEA_LINE_BUF_SIZE          = 100
  constant LORA_AT_BUF_SIZE            = 128
  constant LORA_PAYLOAD_MAX_BYTES      = 80

  # ── Timeouts ────────────────────────────────────────────────────────────────
  constant I2C_TIMEOUT_US              = 2000
  constant UART_RX_TIMEOUT_US          = 5000
  constant LORA_AT_RESPONSE_TIMEOUT_US = 200000   # 200 ms per AT command
  constant LORA_INIT_RETRIES           = 3

  # ── Battery thresholds ──────────────────────────────────────────────────────
  constant BATTERY_LOW_THRESHOLD_DEFAULT  = 20    # percent
  constant BATTERY_CRITICAL_THRESHOLD     = 5     # percent

  # ── LoRa default RF parameters ──────────────────────────────────────────────
  constant LORA_DEFAULT_FREQ_KHZ       = 868000   # 868 MHz (EU ISM)
  constant LORA_DEFAULT_SF             = 9        # Spreading factor
  constant LORA_DEFAULT_POWER_DBM      = 14       # Output power
  constant LORA_DEFAULT_NODE_ADDR      = 1
  constant LORA_GROUND_STATION_ADDR    = 0        # 0 = broadcast

}
