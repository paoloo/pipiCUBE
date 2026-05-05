#include "Top/PipiCubeTopologyAc.hpp"
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>

namespace PipiCube {

static Svc::RateGroupDriver::DividerSet s_rateDivs{{{1, 0}, {4, 0}}};

static U32 s_contexts4Hz[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
static U32 s_contexts1Hz[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    connectComponents();
    regCommands();

    rateGroupDriver.configure(s_rateDivs);
    rateGroup4Hz.configure(s_contexts4Hz, FW_NUM_ARRAY_ELEMENTS(s_contexts4Hz));
    rateGroup1Hz.configure(s_contexts1Hz, FW_NUM_ARRAY_ELEMENTS(s_contexts1Hz));

    startTasks(state);
}

void teardownTopology(const TopologyState& state) {
    stopTasks(state);
    freeThreads(state);
}

} // namespace PipiCube
