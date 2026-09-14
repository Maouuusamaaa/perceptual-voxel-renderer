#include "minecraft/minecraft_world_adapter.hpp"
#include "world/world_truth.hpp"
#include <cassert>
#include <iostream>

using namespace pvr;
using namespace pvr::minecraft;

static MinecraftWorldData fixture() {
    MinecraftWorldData d;
    d.metadata = {"adapter", 99, 16, {"overworld"}};
    d.chunks.push_back({-1, 2, "overworld", {{-2, {{0, 0, 0, 5}, {15, 15, 15, 70000}}}}});
    return d;
}

int main() {
    const auto source = fixture();
    WorldTruth a(source.metadata.seed, source.metadata.sectionSize);
    WorldTruth b(source.metadata.seed, source.metadata.sectionSize);
    MinecraftImportOptions options;
    options.fallbackMaterial = 65535;
    auto ra = MinecraftWorldAdapter::import(source, a, options);
    auto rb = MinecraftWorldAdapter::import(source, b, options);
    assert(ra.ok && rb.ok);
    assert(ra.report.importedBlocks == 2);
    assert(ra.report.fallbackMaterials == 1);
    assert(ra.report.diagnostics.size() == 1);
    assert(a.getBlock({-1, -2, 2}, 0, 0, 0) == 5);
    assert(a.getBlock({-1, -2, 2}, 15, 15, 15) == 65535);
    assert(a.getBlock({-1, -2, 2}, 15, 15, 15) == b.getBlock({-1, -2, 2}, 15, 15, 15));
    assert(a.chunkVersion({-1, -2, 2}) == b.chunkVersion({-1, -2, 2}));

    WorldTruth c(99, 16);
    MinecraftWorldData coords = source;
    coords.chunks[0].x = 3;
    coords.chunks[0].z = -4;
    coords.chunks[0].sections[0].y = 7;
    auto rc = MinecraftWorldAdapter::import(coords, c, options);
    assert(rc.ok);
    assert(c.getBlock({3, 7, -4}, 0, 0, 0) == 5);

    WorldTruth unchanged(99, 16);
    MinecraftWorldData unknown = source;
    unknown.chunks[0].sections[0].blocks[0].sourceStateId = 123456789;
    auto ru = MinecraftWorldAdapter::import(unknown, unchanged, options);
    assert(ru.ok);
    assert(ru.report.fallbackMaterials == 2);
    assert(unchanged.getBlock({-1, -2, 2}, 0, 0, 0) == 65535);

    std::cout << "pvr_minecraft_world_adapter_tests: PASS\n";
}
