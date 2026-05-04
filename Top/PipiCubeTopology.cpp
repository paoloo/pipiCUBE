#include "PipiCubeTopology.hpp"
#include "PipiCubeTopologyAc.hpp"

// Component instances (declared in autocoded TopologyAc.cpp)
#include <Components/BatteryMonitor/BatteryMonitor.hpp>
#include <Components/GpsReceiver/GpsReceiver.hpp>
#include <Components/LoRaDriver/LoRaDriver.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>
#include <Svc/CmdDispatcher/CmdDispatcher.hpp>
#include <Svc/TlmChan/TlmChan.hpp>
#include <Svc/ActiveLogger/ActiveLogger.hpp>
#include <Svc/Time/Time.hpp>

namespace PipiCube {

// ── Static component instances ────────────────────────────────────────────────
// Declared static to avoid dynamic allocation.

static Svc::RateGroupDriver  rateGroupDriver ("rateGroupDriver");
static Svc::ActiveRateGroup  rateGroup1Hz    ("rateGroup1Hz");
static Svc::ActiveRateGroup  rateGroup4Hz    ("rateGroup4Hz");
static Svc::CmdDispatcher    cmdDisp         ("cmdDisp", 20);
static Svc::TlmChan          tlmChan         ("tlmChan");
static Svc::ActiveLogger     eventLogger     ("eventLogger");
static Svc::Time             systemTime      ("systemTime");

static BatteryMonitor        batteryMonitor  ("batteryMonitor");
static GpsReceiver           gpsReceiver     ("gpsReceiver");
static LoRaDriver            loraDriver      ("loraDriver");

// ── Rate group configuration ──────────────────────────────────────────────────
// Hardware timer fires at 4 Hz; dividers subdivide to each rate group.
const NATIVE_UINT_TYPE rateDivs[2]  = {1U, 4U};   // [0]→4Hz, [1]→1Hz
const NATIVE_UINT_TYPE contexts4Hz[1] = {0U};
const NATIVE_UINT_TYPE contexts1Hz[2] = {0U, 1U};

// ── setupTopology ─────────────────────────────────────────────────────────────
void setupTopology(void) {
    // Initialise infrastructure components
    rateGroupDriver.init();
    rateGroup1Hz.init(10, 0);
    rateGroup4Hz.init(10, 0);
    cmdDisp.init(20, 0);
    tlmChan.init(10, 0);
    eventLogger.init(10, 0);
    systemTime.init(0, 0);

    // Initialise application components
    batteryMonitor.init(0);
    gpsReceiver.init(0);
    loraDriver.init(0);

    // Configure rate group driver dividers
    rateGroupDriver.configure(rateDivs,
                               FW_NUM_ARRAY_ELEMENTS(rateDivs));

    // Configure rate group member contexts
    rateGroup1Hz.configure(contexts1Hz, FW_NUM_ARRAY_ELEMENTS(contexts1Hz));
    rateGroup4Hz.configure(contexts4Hz, FW_NUM_ARRAY_ELEMENTS(contexts4Hz));

    // Wire the topology (autocoded function from PipiCubeTopologyAc.cpp)
    initComponents();
    connectComponents();

    // Start active component threads
    rateGroup1Hz.start(0, Svc::ActiveRateGroup::ACTIVE_COMPONENT_EXIT, 90, 4096);
    rateGroup4Hz.start(0, Svc::ActiveRateGroup::ACTIVE_COMPONENT_EXIT, 85, 4096);
    cmdDisp.start(0, Svc::CmdDispatcher::ACTIVE_COMPONENT_EXIT, 70, 4096);
    tlmChan.start(0, Svc::TlmChan::ACTIVE_COMPONENT_EXIT, 60, 4096);
    eventLogger.start(0, Svc::ActiveLogger::ACTIVE_COMPONENT_EXIT, 65, 4096);
    batteryMonitor.start(0, BatteryMonitor::ACTIVE_COMPONENT_EXIT, 80, 2048);
    gpsReceiver.start(0, GpsReceiver::ACTIVE_COMPONENT_EXIT, 75, 2048);
    loraDriver.start(0, LoRaDriver::ACTIVE_COMPONENT_EXIT, 70, 4096);

    // Register commands and run LoRa preamble (init sequence)
    batteryMonitor.regCommands();
    gpsReceiver.regCommands();
    loraDriver.regCommands();
    loraDriver.preamble();
}

// ── teardownTopology ──────────────────────────────────────────────────────────
void teardownTopology(void) {
    // Exit active component threads gracefully
    rateGroup1Hz.exit();
    rateGroup4Hz.exit();
    cmdDisp.exit();
    tlmChan.exit();
    eventLogger.exit();
    batteryMonitor.exit();
    gpsReceiver.exit();
    loraDriver.exit();

    // Join threads (baremetal: no-op unless using RTOS)
    rateGroup1Hz.ActiveComponentBase::join(nullptr);
    rateGroup4Hz.ActiveComponentBase::join(nullptr);
    cmdDisp.ActiveComponentBase::join(nullptr);
    tlmChan.ActiveComponentBase::join(nullptr);
    eventLogger.ActiveComponentBase::join(nullptr);
    batteryMonitor.ActiveComponentBase::join(nullptr);
    gpsReceiver.ActiveComponentBase::join(nullptr);
    loraDriver.ActiveComponentBase::join(nullptr);
}

} // namespace PipiCube
