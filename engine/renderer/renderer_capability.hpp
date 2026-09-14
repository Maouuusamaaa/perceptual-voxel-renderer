#pragma once

namespace pvr {

enum class RendererTier { Tier0, Tier1, Tier2 };

struct RendererFeatureSet {
    bool compute_culling{};
    bool indirect_count{};
    bool descriptor_indexing{};
};

struct RendererCapability {
    RendererTier tier{RendererTier::Tier0};
    bool gpu_culling{};
    bool indirect_count{};
    bool descriptor_indexing{};

    static RendererCapability fromFeatureSet(const RendererFeatureSet& features) noexcept;
};

}
