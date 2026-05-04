#ifndef PIPICUBE_RP2040_UART_H
#define PIPICUBE_RP2040_UART_H

// Internal UART driver header — interrupt-driven RX ring buffer.
// Use rp2040_hal.h for the public API.

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_RING_BUF_SIZE 256U   // must be a power of 2

// Initialise UART0 at 115200 baud on GP0(TX)/GP1(RX).
int32_t uart0_init_hw(void);

// Initialise UART1 at 9600 baud on GP4(TX)/GP5(RX).
int32_t uart1_init_hw(void);

// Blocking write up to `len` bytes; returns bytes written or HAL_ERR_TIMEOUT.
int32_t uart_write(uint8_t uart_id, const uint8_t *buf, size_t len, uint32_t timeout_us);

// Non-blocking read of one byte from the ISR-filled ring buffer.
// Returns HAL_OK + byte, or HAL_ERR_EMPTY.
int32_t uart_getc_nb(uint8_t uart_id, uint8_t *byte);

// Number of bytes ready in the ring buffer.
uint32_t uart_rx_available(uint8_t uart_id);

// Discard all buffered RX bytes.
void uart_rx_flush(uint8_t uart_id);

// IRQ handlers — called by the SDK's IRQ dispatcher.
void uart0_rx_irq_handler(void);
void uart1_rx_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif // PIPICUBE_RP2040_UART_H
