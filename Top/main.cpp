// PipiCube main entry point.
// Initialises all hardware peripherals, sets up the F Prime topology,
// then runs the cooperative dispatch loop with watchdog kicking.

#include "PipiCubeTopology.hpp"
#include <hal/rp2040_hal.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "pico/multicore.h"

// ── Rate group hardware timer ─────────────────────────────────────────────────
// Fires at 4 Hz (250 ms period). Drives Svc.RateGroupDriver.
static volatile bool s_cycle_flag = false;

static bool rate_timer_callback(struct repeating_timer *rt) {
    (void)rt;
    s_cycle_flag = true;
    return true;   // keep repeating
}

// ── Hardware initialisation ───────────────────────────────────────────────────
static void hw_init(void) {
    // Board peripherals
    stdio_init_all();

    // I2C1 for battery monitor (GP6=SDA, GP7=SCL)
    int32_t rc = hal_i2c1_init();
    (void)rc;   // logged via F' events after topology is up

    // UART0 for LoRa (GP0=TX, GP1=RX, 115200)
    rc = hal_uart0_init();
    (void)rc;

    // UART1 for GPS NEO-6M (GP4=TX, GP5=RX, 9600)
    rc = hal_uart1_init();
    (void)rc;

    // Watchdog: 2-second window — must kick at least every 1 second
    hal_watchdog_enable(2000U);
}

int main(void) {
    hw_init();

    // Start F Prime topology (connects components, starts threads)
    PipiCube::setupTopology();

    // Install a hardware repeating timer at 4 Hz
    struct repeating_timer rate_timer;
    add_repeating_timer_ms(250, rate_timer_callback, nullptr, &rate_timer);

    // ── Cooperative main loop ─────────────────────────────────────────────────
    // On the baremetal Os port there is no preemptive scheduler.
    // Active components run via Os::Task::dispatch_all(), called each tick.
    for (;;) {
        // Wait for the 4 Hz tick
        while (!s_cycle_flag) {
            tight_loop_contents();
        }
        s_cycle_flag = false;

        // Kick the watchdog once per cycle (every 250 ms < 2 s window)
        hal_watchdog_kick();

        // Dispatch all registered F' component tasks cooperatively
        Os::Task::dispatch_all();
    }

    // Unreachable, but required for completeness
    PipiCube::teardownTopology();
    return 0;
}
