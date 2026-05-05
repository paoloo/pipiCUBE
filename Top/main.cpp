// PipiCube main entry point.
// Initialises all hardware peripherals, sets up the F Prime topology,
// then drives the rate group driver at 4 Hz from a hardware repeating timer.

#include "PipiCubeTopology.hpp"
#include "Top/PipiCubeTopologyAc.hpp"
#include <Os/RawTime.hpp>
#include <hal/rp2040_hal.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"

static volatile bool s_cycle_flag = false;

static bool rate_timer_callback(struct repeating_timer *rt) {
    (void)rt;
    s_cycle_flag = true;
    return true;
}

static void hw_init(void) {
    stdio_init_all();

    int32_t rc = hal_i2c1_init();
    (void)rc;
    rc = hal_uart0_init();
    (void)rc;
    rc = hal_uart1_init();
    (void)rc;

    hal_watchdog_enable(2000U);
}

int main(void) {
    hw_init();

    PipiCube::TopologyState state;
    PipiCube::setupTopology(state);

    struct repeating_timer rate_timer;
    add_repeating_timer_ms(250, rate_timer_callback, nullptr, &rate_timer);

    Os::RawTime cycleStart;
    for (;;) {
        while (!s_cycle_flag) {
            tight_loop_contents();
        }
        s_cycle_flag = false;

        hal_watchdog_kick();

        cycleStart.now();
        PipiCube::rateGroupDriver.CycleIn_handler(0, cycleStart);
    }

    PipiCube::teardownTopology(state);
    return 0;
}
