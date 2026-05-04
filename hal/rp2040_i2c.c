#include "rp2040_i2c.h"
#include "rp2040_hal.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define I2C1_SDA_PIN  6U
#define I2C1_SCL_PIN  7U
#define I2C1_FREQ_HZ  100000U

int32_t i2c1_init_hw(void) {
    uint32_t actual = i2c_init(i2c1, I2C1_FREQ_HZ);
    if (actual == 0U) {
        return HAL_ERR_BUS;
    }
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    return HAL_OK;
}

int32_t i2c1_read_reg(uint8_t addr, uint8_t reg,
                       uint8_t *buf, size_t len,
                       uint32_t timeout_us) {
    if (buf == NULL || len == 0U) {
        return HAL_ERR_INVAL;
    }

    // Write register pointer with repeated-start (nostop = true)
    int rc = i2c_write_timeout_us(i2c1, addr, &reg, 1, true, (uint32_t)timeout_us);
    if (rc == PICO_ERROR_GENERIC || rc == PICO_ERROR_TIMEOUT) {
        return (rc == PICO_ERROR_TIMEOUT) ? HAL_ERR_TIMEOUT : HAL_ERR_BUS;
    }

    // Read bytes
    rc = i2c_read_timeout_us(i2c1, addr, buf, len, false, (uint32_t)timeout_us);
    if (rc == PICO_ERROR_GENERIC || rc == PICO_ERROR_TIMEOUT) {
        return (rc == PICO_ERROR_TIMEOUT) ? HAL_ERR_TIMEOUT : HAL_ERR_BUS;
    }
    if ((size_t)rc != len) {
        return HAL_ERR_BUS;
    }
    return HAL_OK;
}

int32_t i2c1_write(uint8_t addr, const uint8_t *buf, size_t len, uint32_t timeout_us) {
    if (buf == NULL || len == 0U) {
        return HAL_ERR_INVAL;
    }
    int rc = i2c_write_timeout_us(i2c1, addr, buf, len, false, (uint32_t)timeout_us);
    if (rc == PICO_ERROR_GENERIC || rc == PICO_ERROR_TIMEOUT) {
        return (rc == PICO_ERROR_TIMEOUT) ? HAL_ERR_TIMEOUT : HAL_ERR_BUS;
    }
    return HAL_OK;
}
