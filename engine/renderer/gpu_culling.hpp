#pragma once
#include "renderer/gpu_pipeline.hpp"
#include "renderer/perceptual_culling.hpp"
#include <cstdint>
#include <vector>

namespace pvr {
struct GPUCullingCandidate {
    std::uint64_t node_id{};
    Vec3 position{};
    float radius{1.0f};
    float screen_coverage{};
    float importance{};
    std::uint32_t lod_hint{};
    std::uint32_t geometry_id{};
};

struct GPUDispatchSize { std::uint32_t groups_x{}; std::uint32_t groups_y{1}; std::uint32_t groups_z{1}; };

struct GPUCullingFrame {
    std::vector<GPUCullingCandidate> candidates;
    std::size_t max_draws{0};
    std::uint32_t workgroup_size{64};
};

struct GPUVisibleCandidate : VisibleCandidate { std::uint64_t node_id{}; std::uint32_t geometry_id{}; std::uint32_t material_id{}; };

struct GPUCullingResult {
    std::vector<GPUVisibleCandidate> visible;
    std::vector<GPUIndirectCommand> indirect_commands;
    std::size_t rejected{};
    GPUDispatchSize dispatch{};
};

class GPUCullingPipeline {
public:
    GPUCullingResult execute(const GPUCullingFrame& frame, const ViewFrustum& view) const;
};
}
