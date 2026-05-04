#ifndef PIPICUBE_TOPOLOGY_HPP
#define PIPICUBE_TOPOLOGY_HPP

namespace PipiCube {

// Rate group dividers for the hardware timer → Svc.RateGroupDriver.
// Index 0 → CycleOut[0] fires every 1 tick  = 4 Hz (for LoRa)
// Index 1 → CycleOut[1] fires every 4 ticks = 1 Hz (for battery + GPS)
extern const NATIVE_UINT_TYPE rateDivs[2];

// Context values passed to each rate-group member's schedIn port.
extern const NATIVE_UINT_TYPE contexts4Hz[1];
extern const NATIVE_UINT_TYPE contexts1Hz[2];

// Lifecycle functions called from main.cpp
void setupTopology(void);
void teardownTopology(void);

} // namespace PipiCube

#endif // PIPICUBE_TOPOLOGY_HPP
