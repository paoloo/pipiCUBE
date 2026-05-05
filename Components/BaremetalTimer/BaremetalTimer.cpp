#include "BaremetalTimer.hpp"

namespace PipiCube {

BaremetalTimer::BaremetalTimer(const char* compName)
    : BaremetalTimerComponentBase(compName) {}

BaremetalTimer::~BaremetalTimer() {}

void BaremetalTimer::init(FwEnumStoreType instance) {
    BaremetalTimerComponentBase::init(instance);
}

void BaremetalTimer::tick(Os::RawTime& cycleStart) {
    this->CycleOut_out(0, cycleStart);
}

} // namespace PipiCube
