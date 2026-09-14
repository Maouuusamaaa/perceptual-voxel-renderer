#pragma once

#include "mesh/greedy_mesher.hpp"
#include "mesh/mesh_cluster.hpp"

#include <cstdint>
#include <vector>

namespace pvr {

class MeshClusterBuilder {
public:
    std::vector<MeshCluster> build(
        const Mesh& mesh,
        std::uint32_t quads_per_cluster) const;
};

}
