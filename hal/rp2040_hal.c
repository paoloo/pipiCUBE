#include "rp2040_hal.h"
#include "rp2040_i2c.h"
#include "rp2040_uart.h"

// ── I2C facade ───────────────────────────────────────────────────────────────
int32_t hal_i2c1_init(void) {
    return i2c1_init_hw();
}

int32_t hal_i2c_read_reg(uint8_t i2c_id, uint8_t addr, uint8_t reg,
                          uint8_t *buf, size_t len, uint32_t timeout_us) {
    if (i2c_id != HAL_I2C1) {
        return HAL_ERR_INVAL;   // only I2C1 is wired on this board
    }
    return i2c1_read_reg(addr, reg, buf, len, timeout_us);
}

int32_t hal_i2c_write(uint8_t i2c_id, uint8_t addr,
                      const uint8_t *buf, size_t len, uint32_t timeout_us) {
    if (i2c_id != HAL_I2C1) {
        return HAL_ERR_INVAL;
    }
    return i2c1_write(addr, buf, len, timeout_us);
}

// ── UART facade ──────────────────────────────────────────────────────────────
int32_t hal_uart0_init(void) { return uart0_init_hw(); }
int32_t hal_uart1_init(void) { return uart1_init_hw(); }

int32_t hal_uart_write(uint8_t uart_id, const uint8_t *buf,
                       size_t len, uint32_t timeout_us) {
    return uart_write(uart_id, buf, len, timeout_us);
}

int32_t hal_uart_getc(uint8_t uart_id, uint8_t *byte) {
    return uart_getc_nb(uart_id, byte);
}

uint32_t hal_uart_rx_available(uint8_t uart_id) {
    return uart_rx_available(uart_id);
}

void hal_uart_rx_flush(uint8_t uart_id) {
    uart_rx_flush(uart_id);
}
