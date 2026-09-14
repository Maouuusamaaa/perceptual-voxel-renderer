#include "renderer/renderer_capability.hpp"
#include <cassert>
#include <iostream>

int main() {
    const auto baseline = pvr::RendererCapability::fromFeatureSet({false, false, false});
    assert(baseline.tier == pvr::RendererTier::Tier0);
    assert(!baseline.gpu_culling);
    assert(!baseline.indirect_count);

    const auto capable = pvr::RendererCapability::fromFeatureSet({true, true, false});
    assert(capable.tier == pvr::RendererTier::Tier1);
    assert(capable.gpu_culling);
    assert(capable.indirect_count);

    const auto high = pvr::RendererCapability::fromFeatureSet({true, true, true});
    assert(high.tier == pvr::RendererTier::Tier2);
    assert(high.descriptor_indexing);
    std::cout << "renderer_capability_tests: PASS\n";
}
