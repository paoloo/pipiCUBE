#ifndef PIPICUBE_RP2040_HAL_H
#define PIPICUBE_RP2040_HAL_H

// Public HAL API for PipiCube RP2040 firmware.
// All functions are reentrant-safe when called from a single thread.
// All return values are [[nodiscard]] — callers MUST check them (SEI CERT ERR33-C).

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Return codes ─────────────────────────────────────────────────────────────
#define HAL_OK          ( 0)
#define HAL_ERR_TIMEOUT (-1)
#define HAL_ERR_BUS     (-2)
#define HAL_ERR_INVAL   (-3)
#define HAL_ERR_EMPTY   (-4)   // non-blocking read: no byte available
#define HAL_ERR_FULL    (-5)   // write buffer full

// ── Bus IDs ──────────────────────────────────────────────────────────────────
#define HAL_I2C0   0U
#define HAL_I2C1   1U
#define HAL_UART0  0U   // LoRa: GP0=TX, GP1=RX
#define HAL_UART1  1U   // GPS:  GP4=TX, GP5=RX

// ── HAL initialisation ───────────────────────────────────────────────────────

// Initialise I2C1 at 100 kHz on GP6(SDA)/GP7(SCL).
// Must be called once during system startup.
__attribute__((warn_unused_result))
int32_t hal_i2c1_init(void);

// Initialise UART0 at 115200-8-N-1 on GP0(TX)/GP1(RX) for LoRa.
__attribute__((warn_unused_result))
int32_t hal_uart0_init(void);

// Initialise UART1 at 9600-8-N-1 on GP4(TX)/GP5(RX) for GPS.
__attribute__((warn_unused_result))
int32_t hal_uart1_init(void);

// ── I2C ─────────────────────────────────────────────────────────────────────

// Write `reg` then read `len` bytes into `buf` from device at `addr`.
// `i2c_id` must be HAL_I2C1 (only I2C1 is wired on this board).
// Returns HAL_OK or a HAL_ERR_* code.
__attribute__((warn_unused_result))
int32_t hal_i2c_read_reg(uint8_t i2c_id,
                          uint8_t addr,
                          uint8_t reg,
                          uint8_t *buf,
                          size_t   len,
                          uint32_t timeout_us);

// Write `len` bytes from `buf` to device at `addr`.
__attribute__((warn_unused_result))
int32_t hal_i2c_write(uint8_t        i2c_id,
                      uint8_t        addr,
                      const uint8_t *buf,
                      size_t         len,
                      uint32_t       timeout_us);

// ── UART ─────────────────────────────────────────────────────────────────────

// Write `len` bytes from `buf` to UART `uart_id`.
// Blocks until all bytes are queued in the TX FIFO or `timeout_us` elapses.
__attribute__((warn_unused_result))
int32_t hal_uart_write(uint8_t        uart_id,
                       const uint8_t *buf,
                       size_t         len,
                       uint32_t       timeout_us);

// Non-blocking read of a single byte from the HAL ring buffer.
// Returns HAL_OK and writes to *byte, or HAL_ERR_EMPTY if no data.
__attribute__((warn_unused_result))
int32_t hal_uart_getc(uint8_t uart_id, uint8_t *byte);

// Returns the number of bytes available in the RX ring buffer.
uint32_t hal_uart_rx_available(uint8_t uart_id);

// Flush the RX ring buffer (discard all buffered bytes).
void hal_uart_rx_flush(uint8_t uart_id);

// ── Time ─────────────────────────────────────────────────────────────────────

// Returns microseconds since boot (wraps at UINT64_MAX ~= 584 000 years).
uint64_t hal_time_us(void);

// Busy-wait for exactly `ms` milliseconds.
// Never call with ms > 1000 — use a loop if longer delays are needed.
void hal_delay_ms(uint32_t ms);

// ── Watchdog ─────────────────────────────────────────────────────────────────

// Enable the hardware watchdog with a `timeout_ms` window.
// Must be kicked by hal_watchdog_kick() within each window.
void hal_watchdog_enable(uint32_t timeout_ms);

// Kick (reset) the watchdog timer.
void hal_watchdog_kick(void);

#ifdef __cplusplus
}
#endif

#endif // PIPICUBE_RP2040_HAL_H
