#include "renderer/gpu_culling.hpp"
#include <algorithm>
#include <cmath>

namespace pvr {
GPUCullingResult GPUCullingPipeline::execute(const GPUCullingFrame& frame, const ViewFrustum& view) const {
    GPUCullingResult out;
    const std::uint32_t wg = std::max<std::uint32_t>(1, frame.workgroup_size);
    out.dispatch.groups_x = static_cast<std::uint32_t>((frame.candidates.size() + wg - 1) / wg);
    std::vector<PerceptualCandidate> candidates;
    candidates.reserve(frame.candidates.size());
    for (const auto& c : frame.candidates) {
        candidates.push_back({c.position, c.radius, c.screen_coverage, c.importance, false});
    }
    PerceptualCulling culling;
    const std::size_t budget = frame.max_draws == 0 ? candidates.size() : frame.max_draws;
    auto result = culling.evaluate(candidates, view, budget);
    out.rejected = result.rejected;
    out.visible.reserve(result.visible.size());
    out.indirect_commands.reserve(result.visible.size());
    for (const auto& visible : result.visible) {
        GPUVisibleCandidate gpu_visible{};
        static_cast<VisibleCandidate&>(gpu_visible) = visible;
        std::uint64_t node_id = 0;
        std::uint32_t geometry_id = 0;
        for (const auto& c : frame.candidates) {
            if (c.position.x == visible.position.x && c.position.y == visible.position.y && c.position.z == visible.position.z) {
                node_id = c.node_id;
                geometry_id = c.geometry_id;
                break;
            }
        }
        gpu_visible.node_id = node_id;
        gpu_visible.geometry_id = geometry_id;
        out.visible.push_back(gpu_visible);
        out.indirect_commands.push_back({node_id, geometry_id, 0, static_cast<std::uint32_t>(visible.lod), 1});
    }
    return out;
}
}
