#ifndef PIPICUBE_TOPOLOGYDEFS_HPP
#define PIPICUBE_TOPOLOGYDEFS_HPP

#include "Top/FppConstantsAc.hpp"

namespace PipiCube {
    // No runtime parameters needed on bare-metal; empty state satisfies the
    // topology autocoder's requirement for a TopologyState type.
    struct TopologyState {};
}

#endif
