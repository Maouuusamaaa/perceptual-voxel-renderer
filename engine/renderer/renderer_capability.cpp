#include "renderer/renderer_capability.hpp"

namespace pvr {

RendererCapability RendererCapability::fromFeatureSet(const RendererFeatureSet& features) noexcept {
    RendererCapability result;
    result.gpu_culling = features.compute_culling;
    result.indirect_count = features.indirect_count;
    result.descriptor_indexing = features.descriptor_indexing;
    if (features.compute_culling && features.indirect_count && features.descriptor_indexing) {
        result.tier = RendererTier::Tier2;
    } else if (features.compute_culling && features.indirect_count) {
        result.tier = RendererTier::Tier1;
    } else {
        result.tier = RendererTier::Tier0;
    }
    return result;
}

}
