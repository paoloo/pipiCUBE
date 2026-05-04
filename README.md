# PipiCube

Nanosat firmware for the Raspberry Pi Pico (RP2040) built on [NASA F Prime (F')](https://github.com/nasa/fprime).

The firmware monitors battery state, acquires GPS position, and downlinks telemetry over LoRa — all structured as typed, commanded F' components with telemetry channels, events, and rate-group scheduling.

---

## Hardware

| Role | Part | Interface | Pins |
|---|---|---|---|
| Microcontroller | Raspberry Pi Pico (RP2040) | — | — |
| UPS / Battery | [Seeed Pico-UPS-A](https://mauser.pt/096-9897/seeed-pico-ups-a-ups-para-raspberry-pi-pico) (IP5306) | I2C1, 100 kHz | GP6 = SDA, GP7 = SCL |
| GPS | NEO-6M V2 | UART1, 9600-8-N-1 | GP4 = TX, GP5 = RX |
| LoRa radio | RYLR896-compatible (AT command) | UART0, 115200-8-N-1 | GP0 = TX, GP1 = RX |

> The LoRa module is assumed to use the REYAX RYLR896 AT command set (also compatible with RYLR406, E22-900T22S, and similar UART LoRa modules).

---

## Architecture

The firmware is structured as three F' active components wired into a single topology:

```
                       ┌──────────────────────────────────────────┐
                       │              F' Topology                  │
                       │                                           │
  4 Hz HW timer ──► RateGroupDriver ──► RateGroup4Hz ──► LoRaDriver (UART0)
                                    └──► RateGroup1Hz ──► BatteryMonitor (I2C1)
                                                      └──► GpsReceiver (UART1)
                       │                                           │
                       │  CmdDispatcher ◄── (LoRa downlink cmds)  │
                       │  TlmChan       ◄── all component tlm      │
                       │  EventLogger   ◄── all component events   │
                       └──────────────────────────────────────────┘
```

### Components

#### `PipiCube.BatteryMonitor`
Polls the IP5306 battery management IC via I2C1 at 1 Hz.

| Telemetry | ID | Type | Description |
|---|---|---|---|
| `BatteryPercent` | 0x100 | U8 | Charge level % (4-step: 12/37/62/88/100) |
| `ChargeState` | 0x101 | U8 | 0=idle, 1=pre-charge, 2=CC, 3=CV, 4=done |
| `OutputEnabled` | 0x102 | bool | Boost converter output state |
| `I2cErrors` | 0x103 | U32 | Cumulative I2C error count |

| Command | Opcode | Description |
|---|---|---|
| `BATTERY_POLL` | 0x00 | Force an immediate out-of-schedule read |
| `BATTERY_SET_LOW_THRESHOLD` | 0x01 | Set low-battery warning level (1–99%) |

Events: `BatteryLow` (warning), `BatteryCritical` (fatal), `ChargeStateChanged`, `BatteryFull`, `I2cReadError`.

---

#### `PipiCube.GpsReceiver`
Reads NMEA GGA sentences from the NEO-6M V2 via UART1 at 1 Hz. RX is interrupt-driven into a 256-byte ring buffer; parsing runs in-task.

| Telemetry | ID | Type | Description |
|---|---|---|---|
| `FixQuality` | 0x200 | U8 | 0=invalid, 1=GPS, 2=DGPS |
| `SatCount` | 0x201 | U8 | Satellites used in fix |
| `Latitude_1e6deg` | 0x202 | I32 | Latitude × 10⁶ (negative = South) |
| `Longitude_1e6deg` | 0x203 | I32 | Longitude × 10⁶ (negative = West) |
| `Altitude_cm` | 0x204 | I32 | Altitude above MSL in cm |
| `HDOP_x100` | 0x205 | U16 | HDOP × 100 (e.g. 1.2 → 120) |
| `UtcTime_hms` | 0x206 | U32 | Packed UTC: `(hh<<16)\|(mm<<8)\|ss` |
| `SentenceCount` | 0x207 | U32 | Valid GGA sentences since boot |

| Command | Opcode | Description |
|---|---|---|
| `GPS_RESET` | 0x10 | Flush ring buffer and reset line assembler |
| `GPS_SET_ENABLED` | 0x11 | Enable / disable telemetry output |

Events: `FixAcquired`, `FixLost`, `NmeaChecksumError`, `UartOverrun`, `ParseError`.

---

#### `PipiCube.LoRaDriver`
Drives the LoRa module via AT commands on UART0. Polls for `+RCV=` responses at 4 Hz. Accepts outbound payloads (≤ 80 bytes ASCII) via the `dataIn` port.

| Telemetry | ID | Type | Description |
|---|---|---|---|
| `LastRxRssi_dBm` | 0x300 | I16 | Last RX RSSI in dBm |
| `LastRxSnr_x10` | 0x301 | I16 | Last RX SNR × 10 |
| `TxPacketCount` | 0x302 | U32 | Uplink packets sent since boot |
| `TxErrorCount` | 0x303 | U32 | TX operations that returned `+ERR` |
| `RxPacketCount` | 0x304 | U32 | Downlink packets received |
| `ModuleState` | 0x305 | U8 | 0=uninit, 1=ready, 2=error |

| Command | Opcode | Description |
|---|---|---|
| `LORA_SEND_AT` | 0x20 | Send a raw AT command string |
| `LORA_SET_FREQ` | 0x21 | Set frequency in kHz (e.g. 868000) |
| `LORA_SET_SF` | 0x22 | Set spreading factor [7–12] |
| `LORA_SET_POWER` | 0x23 | Set TX power [0–22 dBm] |
| `LORA_RESET` | 0x24 | Reset and re-initialise the module |

Default RF parameters: 868 MHz, SF9, BW125, CR4/5, 14 dBm. Adjust for your regional ISM band.

---

## Coding Standards

This project follows NASA/JPL and SEI CERT C++ rules applicable to space software:

- **No dynamic allocation after init** — heap size is set to zero in the linker script (`PICO_HEAP_SIZE=0`); all buffers and queues are statically allocated from a pool
- **No recursion** — all handlers are flat; enforced by `-Wstack-usage=1024`
- **No VLAs** — enforced by `-Wvla`
- **Bounded loops** — every loop that reads from a buffer has a hard ceiling equal to the buffer size constant
- **All return codes checked** — HAL functions carry `[[nodiscard]]`/`warn_unused_result`; no silent discard
- **Stack canaries** — `-fstack-protector-strong` is enabled
- **Watchdog** — 2-second hardware watchdog, kicked every 250 ms in the main loop

---

## Prerequisites

| Tool | Version | Notes |
|---|---|---|
| `arm-none-eabi-gcc` | ≥ 12 | [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| `cmake` | ≥ 3.26 | |
| `python3` | ≥ 3.8 | Required by F' build system |
| `fprime-tools` | ≥ 3.4 | `pip install fprime-tools` |
| Pico SDK | ≥ 1.5 | [raspberrypi/pico-sdk](https://github.com/raspberrypi/pico-sdk) |

Install the Arm toolchain on macOS:
```bash
brew install --cask gcc-arm-embedded
```

Install the Arm toolchain on Ubuntu/Debian:
```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

Install F' tools:
```bash
pip install fprime-tools
```

---

## Building

### 1. Clone and initialise submodules

```bash
git clone <this-repo> pipicube
cd pipicube
git submodule update --init --recursive   # pulls fprime/
```

### 2. Set environment variables

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
# Optional: point to a local F' clone instead of the submodule
# export FPRIME_FRAMEWORK_PATH=/path/to/fprime
```

### 3. Generate the build system

```bash
fprime-util generate -DCMAKE_BUILD_TYPE=MinSizeRel
```

This runs `cmake` with the `fprime-arm-cross.cmake` toolchain, invokes the F' FPP autocoder on all `.fpp` model files, and produces the `build-artifacts/` directory.

### 4. Build

```bash
fprime-util build
```

On success you will find:

```
build-artifacts/Top/pipicube.elf
build-artifacts/Top/pipicube.uf2
build-artifacts/Top/pipicube.map
```

### 5. Flash

Hold the **BOOTSEL** button on the Pico while plugging in USB. The Pico mounts as a mass-storage device. Copy the UF2 file:

```bash
cp build-artifacts/Top/pipicube.uf2 /Volumes/RPI-RP2/
# or on Linux:
cp build-artifacts/Top/pipicube.uf2 /media/$USER/RPI-RP2/
```

The board resets and boots automatically after the copy completes.

---

## Alternative: manual CMake build

If you prefer to drive CMake directly without `fprime-util`:

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../fprime-arm-cross.cmake \
  -DPICO_SDK_PATH=$PICO_SDK_PATH \
  -DCMAKE_BUILD_TYPE=MinSizeRel
make -j$(nproc)
```

---

## Project layout

```
pipicube/
├── CMakeLists.txt              # Root build file
├── settings.ini                # fprime-util project settings
├── fprime-arm-cross.cmake      # ARM Cortex-M0+ cross-compilation toolchain
├── cmake/
│   └── rp2040.cmake            # RP2040-specific compile flags
├── config/
│   ├── FpConfig.h              # F' integer types and framework tuning
│   └── AcConstants.fpp         # Project constants (queue sizes, RF params, etc.)
├── hal/                        # Hardware abstraction layer
│   ├── rp2040_hal.h/c          # Public API
│   ├── rp2040_i2c.h/c          # I2C1 driver (pico-sdk wrapper)
│   ├── rp2040_uart.h/c         # UART0/1 with ISR-driven ring buffers
│   └── rp2040_time.c           # Monotonic time and watchdog
├── Os/
│   └── Baremetal/              # F' Os port: cooperative scheduler, no RTOS
│       ├── Task.cpp            # Round-robin task table
│       ├── Mutex.cpp           # Interrupt-disable mutex
│       ├── Queue.cpp           # Static ring-buffer queue pool
│       └── File.cpp            # No-filesystem stub
├── Components/
│   ├── BatteryMonitor/         # IP5306 battery monitor (FPP + C++)
│   ├── GpsReceiver/            # NEO-6M NMEA parser (FPP + C++)
│   └── LoRaDriver/             # AT-command LoRa driver (FPP + C++)
└── Top/
    ├── PipiCubeTopology.fpp    # Component instances and port connections
    ├── PipiCubeTopology.hpp/cpp # Hand-written lifecycle (init, start, teardown)
    └── main.cpp                # Entry point: hw_init → topology → dispatch loop
```

---

## Rate groups and scheduling

The RP2040 hardware timer fires at **4 Hz** (250 ms). A `Svc.RateGroupDriver` divides this into:

| Rate group | Frequency | Members |
|---|---|---|
| `rateGroup4Hz` | 4 Hz | `LoRaDriver.schedIn` (polls for `+RCV=`) |
| `rateGroup1Hz` | 1 Hz | `BatteryMonitor.schedIn`, `GpsReceiver.schedIn` |

The main loop kicks the hardware watchdog and calls `Os::Task::dispatch_all()` on every 250 ms tick. There is no preemptive scheduler — all components must complete their handler and return; long-running work is forbidden.

---

## Telemetry downlink format

The `LoRaDriver` accepts ASCII payloads (≤ 80 bytes) via its `dataIn` port. The recommended format assembled by the application layer is:

```
BAT:<pct>%;<state> GPS:<lat>,<lon>,<alt>;<sats> T:<hms>
```

Example:
```
BAT:62%;2 GPS:+38123456,-009123456,+5440;08 T:123519
```

This fits comfortably within the SF9/BW125 LoRa payload budget (~222 bytes at SF9).

---

## Changing the LoRa frequency

The default is **868 MHz** (EU ISM band 863–870 MHz). To change it, send the `LORA_SET_FREQ` command with the desired frequency in kHz, or edit `LORA_DEFAULT_FREQ_KHZ` in `config/AcConstants.fpp` before building:

| Region | Frequency |
|---|---|
| EU (default) | 868000 kHz |
| US (915 MHz) | 915000 kHz |
| AS (433 MHz) | 433000 kHz |

Always verify local spectrum regulations before operating.

---

## License

This project is released under the Apache 2.0 License. NASA F Prime is also Apache 2.0. The Raspberry Pi Pico SDK is BSD 3-Clause.
