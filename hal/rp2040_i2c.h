#ifndef PIPICUBE_RP2040_I2C_H
#define PIPICUBE_RP2040_I2C_H

// Internal I2C driver header — use rp2040_hal.h for the public API.

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialise I2C1 peripheral (GP6=SDA, GP7=SCL, 100 kHz).
// Returns HAL_OK or HAL_ERR_BUS.
int32_t i2c1_init_hw(void);

// Write register pointer, then burst-read `len` bytes.
// Returns bytes read (>= 0) or a HAL_ERR_* code (< 0).
int32_t i2c1_read_reg(uint8_t  addr,
                       uint8_t  reg,
                       uint8_t *buf,
                       size_t   len,
                       uint32_t timeout_us);

// Burst-write `len` bytes starting with register address in buf[0].
int32_t i2c1_write(uint8_t        addr,
                   const uint8_t *buf,
                   size_t         len,
                   uint32_t       timeout_us);

#ifdef __cplusplus
}
#endif

#endif // PIPICUBE_RP2040_I2C_H
