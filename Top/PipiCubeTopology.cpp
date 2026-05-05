#include "PipiCubeTopologyAc.hpp"
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>

namespace PipiCube {

// Rate group driver: base tick is 4 Hz; divider 1 → 4 Hz, divider 4 → 1 Hz.
static Svc::RateGroupDriver::DividerSet s_rateDivs{{{1, 0}, {4, 0}}};

// Context tokens passed to rate-group members via schedIn; unused, so all zero.
static U32 s_contexts4Hz[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
static U32 s_contexts1Hz[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

void setupTopology() {
    initComponents();     // autocoded: calls init() on every component
    setBaseIds();         // autocoded: applies FPP base IDs
    connectComponents();  // autocoded: wires all port connections
    regCommands();        // autocoded: registers commands with cmdDisp

    // Configure rate group driver and rate group member-context arrays.
    rateGroupDriver.configure(s_rateDivs);
    rateGroup4Hz.configure(s_contexts4Hz, FW_NUM_ARRAY_ELEMENTS(s_contexts4Hz));
    rateGroup1Hz.configure(s_contexts1Hz, FW_NUM_ARRAY_ELEMENTS(s_contexts1Hz));

    startTasks();         // autocoded: starts all active component threads
}

void teardownTopology() {
    stopTasks();          // autocoded: signals all active components to exit
    freeThreads();        // autocoded: joins all threads
}

} // namespace PipiCube
