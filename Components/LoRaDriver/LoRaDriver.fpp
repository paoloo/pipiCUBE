module PipiCube {

  @ Active component that drives the LoRa UART module (RYLR896-compatible)
  @ via AT commands on UART0 (GP0=TX, GP1=RX, 115200-8-N-1).
  @ Accepts outgoing payload buffers via dataIn and polls for +RCV at 4 Hz.
  active component LoRaDriver {

    # ── Standard F' infrastructure ports ──────────────────────────────────────
    command recv port cmdIn
    command reg  port cmdRegOut
    command resp port cmdResponseOut

    event      port logOut
    text event port logTextOut
    time get   port timeCaller
    telemetry  port tlmOut

    # ── Scheduling (4 Hz — poll for incoming RCV responses) ──────────────────
    sync input port schedIn: Svc.Sched

    # ── Data input: other components push ASCII payload strings (≤ 80 bytes) ─
    async input port dataIn: Fw.BufferSend

    # ── Telemetry channels ────────────────────────────────────────────────────

    @ RSSI of the last received packet in dBm (signed)
    telemetry LastRxRssi_dBm: I16 id 0x300 update on change

    @ SNR of the last received packet × 10 (e.g. 7.5 dB → 75, signed)
    telemetry LastRxSnr_x10: I16 id 0x301 update on change

    @ Uplink packets successfully transmitted since boot
    telemetry TxPacketCount: U32 id 0x302 update always

    @ Transmit operations that returned AT+ERR
    telemetry TxErrorCount: U32 id 0x303 update on change

    @ Downlink packets received since boot
    telemetry RxPacketCount: U32 id 0x304 update always

    @ Module state: 0=uninitialized, 1=ready, 2=error
    telemetry ModuleState: U8 id 0x305 update on change

    # ── Events ────────────────────────────────────────────────────────────────

    @ Module failed to respond to AT during init after LORA_INIT_RETRIES attempts
    event InitFailed(
      attempts: U8
    ) severity fatal id 0x300 \
      format "LoRa init failed after {} attempts"

    @ AT+SEND returned +ERR
    event TxError(
      errCode: U8
    ) severity warning high id 0x301 \
      format "LoRa TX error code {}"

    @ Module successfully initialised and RF parameters confirmed
    event ModuleReady severity activity high id 0x302 \
      format "LoRa module ready"

    @ Downlink packet received
    event PacketReceived(
      rssi: I16
      snr:  I16
    ) severity activity low id 0x303 \
      format "LoRa RX rssi={}dBm snr={}×10"

    @ AT response buffer overran — response discarded
    event AtBufOverrun severity warning high id 0x304 \
      format "LoRa AT response buffer overrun"

    @ Payload pushed to dataIn exceeded LORA_PAYLOAD_MAX_BYTES and was dropped
    event PayloadTooLarge(
      payloadSize: U32
    ) severity warning high id 0x305 \
      format "LoRa payload too large: {} bytes (max 80)"

    # ── Commands ──────────────────────────────────────────────────────────────

    @ Send a raw AT command string (for factory testing and diagnostics)
    async command LORA_SEND_AT(
      cmd: string size 64
    ) opcode 0x20

    @ Set RF centre frequency in kHz (e.g. 868000 for 868 MHz EU)
    async command LORA_SET_FREQ(
      freqKHz: U32
    ) opcode 0x21

    @ Set spreading factor [7–12]
    async command LORA_SET_SF(
      sf: U8
    ) opcode 0x22

    @ Set transmit output power [0–22 dBm]
    async command LORA_SET_POWER(
      powerDbm: U8
    ) opcode 0x23

    @ Issue AT+RESET to the LoRa module and re-run the init sequence
    async command LORA_RESET opcode 0x24

  }

}
