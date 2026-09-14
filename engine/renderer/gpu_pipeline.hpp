#pragma once
#include "gpu/gpu_scene.hpp"
#include "mesh/mesh_cluster.hpp"
#include "renderer/gpu_draw.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace pvr {
struct GPUIndirectCommand {
    std::uint64_t node_id{};
    std::uint32_t geometry_id{};
    std::uint32_t material_id{};
    std::uint32_t lod{};
    std::uint32_t instance_count{};
};

class GPUCommandBuilder {
public:
    void add(const GPUInstance& instance, bool visible);
    std::vector<GPUIndirectCommand> build() const;
    GPUIndirectBatch build_batch() const;
    GPUIndirectBatch build_batch(
        const std::vector<MeshCluster>& clusters
    ) const;

    std::optional<IndirectBuffer> build_indirect_buffer(
        const std::vector<MeshCluster>& clusters
    ) const;

    void clear();
private:
    std::vector<GPUIndirectCommand> commands_;
};
}
