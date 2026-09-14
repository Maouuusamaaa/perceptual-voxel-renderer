#pragma once
#include "renderer/gpu_culling.hpp"
#include "renderer/hierarchical_visibility.hpp"
#include "renderer/temporal_visibility.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace pvr {
struct PipelineCandidate { std::uint64_t node_id{}; Vec3 position{}; float radius{1}; float screen_coverage{}; float importance{}; std::uint32_t geometry_id{}; std::uint32_t material_id{}; };
struct PipelineResult { std::vector<GPUVisibleCandidate> visible; std::size_t rejected{}; };
class PerceptualPipeline {
public:
    explicit PerceptualPipeline(std::size_t temporal_ttl=3): temporal_(temporal_ttl) {}
    void add(const PipelineCandidate& c);
    PipelineResult execute(const ViewFrustum& view, std::size_t max_draws);
    void clear();
private:
    std::vector<PipelineCandidate> candidates_;
    TemporalVisibilityCache temporal_;
};
}
