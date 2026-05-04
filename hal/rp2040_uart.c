#include "rp2040_uart.h"
#include "rp2040_hal.h"

#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/critical_section.h"

// ── UART0 (LoRa): GP0=TX, GP1=RX, 115200 baud ────────────────────────────
#define UART0_TX_PIN   0U
#define UART0_RX_PIN   1U
#define UART0_BAUD     115200U

// ── UART1 (GPS): GP4=TX, GP5=RX, 9600 baud ───────────────────────────────
#define UART1_TX_PIN   4U
#define UART1_RX_PIN   5U
#define UART1_BAUD     9600U

// ── Ring buffer ──────────────────────────────────────────────────────────────
// Power-of-2 size allows cheap modulo via bitmask.
_Static_assert((UART_RING_BUF_SIZE & (UART_RING_BUF_SIZE - 1U)) == 0U,
               "UART_RING_BUF_SIZE must be a power of 2");
#define RING_MASK (UART_RING_BUF_SIZE - 1U)

typedef struct {
    volatile uint8_t  buf[UART_RING_BUF_SIZE];
    volatile uint32_t head;   // write index (IRQ side)
    volatile uint32_t tail;   // read index  (task side)
} RingBuf;

static RingBuf s_rx0;
static RingBuf s_rx1;

static critical_section_t s_cs0;
static critical_section_t s_cs1;

// ── IRQ handlers ─────────────────────────────────────────────────────────────
void uart0_rx_irq_handler(void) {
    while (uart_is_readable(uart0)) {
        uint8_t byte = (uint8_t)uart_getc(uart0);
        uint32_t next = (s_rx0.head + 1U) & RING_MASK;
        if (next != s_rx0.tail) {   // drop on overflow to preserve latest bytes
            s_rx0.buf[s_rx0.head] = byte;
            s_rx0.head = next;
        }
    }
}

void uart1_rx_irq_handler(void) {
    while (uart_is_readable(uart1)) {
        uint8_t byte = (uint8_t)uart_getc(uart1);
        uint32_t next = (s_rx1.head + 1U) & RING_MASK;
        if (next != s_rx1.tail) {
            s_rx1.buf[s_rx1.head] = byte;
            s_rx1.head = next;
        }
    }
}

// ── Init ─────────────────────────────────────────────────────────────────────
int32_t uart0_init_hw(void) {
    critical_section_init(&s_cs0);
    s_rx0.head = 0U;
    s_rx0.tail = 0U;

    uart_init(uart0, UART0_BAUD);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, true);

    irq_set_exclusive_handler(UART0_IRQ, uart0_rx_irq_handler);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);   // RX interrupt only
    return HAL_OK;
}

int32_t uart1_init_hw(void) {
    critical_section_init(&s_cs1);
    s_rx1.head = 0U;
    s_rx1.tail = 0U;

    uart_init(uart1, UART1_BAUD);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, true);

    irq_set_exclusive_handler(UART1_IRQ, uart1_rx_irq_handler);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false);
    return HAL_OK;
}

// ── Write ─────────────────────────────────────────────────────────────────────
int32_t uart_write(uint8_t uart_id, const uint8_t *buf, size_t len, uint32_t timeout_us) {
    if (buf == NULL) {
        return HAL_ERR_INVAL;
    }
    uart_inst_t *uart = (uart_id == HAL_UART0) ? uart0 : uart1;
    uint64_t deadline = hal_time_us() + (uint64_t)timeout_us;

    for (size_t i = 0U; i < len; i++) {
        // Wait until TX FIFO has space
        while (!uart_is_writable(uart)) {
            if (hal_time_us() >= deadline) {
                return HAL_ERR_TIMEOUT;
            }
        }
        uart_putc_raw(uart, (char)buf[i]);
    }
    return HAL_OK;
}

// ── Read (non-blocking) ───────────────────────────────────────────────────────
int32_t uart_getc_nb(uint8_t uart_id, uint8_t *byte) {
    if (byte == NULL) {
        return HAL_ERR_INVAL;
    }
    RingBuf *rb = (uart_id == HAL_UART0) ? &s_rx0 : &s_rx1;

    // Atomically sample head to avoid tearing on 32-bit read
    uint32_t head = rb->head;
    if (rb->tail == head) {
        return HAL_ERR_EMPTY;
    }
    *byte = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1U) & RING_MASK;
    return HAL_OK;
}

uint32_t uart_rx_available(uint8_t uart_id) {
    RingBuf *rb = (uart_id == HAL_UART0) ? &s_rx0 : &s_rx1;
    return (rb->head - rb->tail) & RING_MASK;
}

void uart_rx_flush(uint8_t uart_id) {
    RingBuf *rb = (uart_id == HAL_UART0) ? &s_rx0 : &s_rx1;
    rb->tail = rb->head;
}
