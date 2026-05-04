#include "rp2040_hal.h"
#include "pico/time.h"
#include "hardware/watchdog.h"

uint64_t hal_time_us(void) {
    return time_us_64();
}

void hal_delay_ms(uint32_t ms) {
    // Clamp to 1000 ms — longer delays must use a loop (per coding standard).
    if (ms > 1000U) {
        ms = 1000U;
    }
    sleep_ms(ms);
}

void hal_watchdog_enable(uint32_t timeout_ms) {
    // pico-sdk watchdog: pause_on_debug=true keeps it quiet during JTAG
    watchdog_enable(timeout_ms, true);
}

void hal_watchdog_kick(void) {
    watchdog_update();
}
