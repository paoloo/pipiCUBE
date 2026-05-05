#ifndef PIPICUBE_TOPOLOGY_HPP
#define PIPICUBE_TOPOLOGY_HPP

namespace PipiCube {
    struct TopologyState;  // defined in PipiCubeTopologyDefs.hpp
    void setupTopology(const TopologyState& state);
    void teardownTopology(const TopologyState& state);
}

#endif // PIPICUBE_TOPOLOGY_HPP
