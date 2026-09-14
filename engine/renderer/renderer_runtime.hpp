#pragma once
#include "renderer/perceptual_pipeline.hpp"
#include "renderer/vulkan_backend.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
namespace pvr {
enum class RenderMode { Vulkan, CPUFallback };
struct RendererInitResult { RenderMode mode{RenderMode::CPUFallback}; bool hardware_accelerated{}; std::string message; };
struct RenderFrameResult { std::size_t visible_count{}; std::size_t draw_count{}; std::size_t rejected{}; std::vector<GPUIndirectCommand> indirect_commands; };
class RendererRuntime {
public:
    RendererInitResult initialize();
    void add_candidate(const PipelineCandidate& c);
    RenderFrameResult render(const ViewFrustum& view, std::size_t max_draws);

    std::optional<IndirectBuffer> build_indirect_buffer(
        const RenderFrameResult& frame,
        const std::vector<MeshCluster>& clusters
    );
    RenderMode mode() const noexcept { return mode_; }
private:
    RenderMode mode_{RenderMode::CPUFallback};
    VulkanBackend vulkan_;
    PerceptualPipeline pipeline_;
};
}
