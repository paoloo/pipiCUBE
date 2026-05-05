#ifndef PIPICUBE_BAREMETAL_TIMER_HPP
#define PIPICUBE_BAREMETAL_TIMER_HPP

#include <Components/BaremetalTimer/BaremetalTimerComponentAc.hpp>
#include <Os/RawTime.hpp>

namespace PipiCube {

class BaremetalTimer final : public BaremetalTimerComponentBase {
  public:
    explicit BaremetalTimer(const char* compName);
    ~BaremetalTimer();

    void init(FwEnumStoreType instance = 0);

    // Call from the main loop after the hardware timer fires.
    // Invokes CycleOut_out which dispatches to rateGroupDriver.CycleIn.
    void tick(Os::RawTime& cycleStart);
};

} // namespace PipiCube

#endif // PIPICUBE_BAREMETAL_TIMER_HPP
