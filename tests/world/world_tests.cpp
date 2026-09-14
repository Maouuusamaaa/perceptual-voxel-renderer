#include "world/world_truth.hpp"
#include "spatial/spatial_hierarchy.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace pvr;

    WorldTruth world(1234, 8);
    assert(world.seed() == 1234);

    ChunkId a{10, -20, 3};
    ChunkId b{10, -20, 3};
    assert(a == b);
    assert(a.toString() == "10,-20,3");

    world.setBlock(a, 1, 2, 3, 7);
    assert(world.getBlock(a, 1, 2, 3) == 7);
    assert(world.isDirty(a));
    const auto v1 = world.chunkVersion(a);
    world.clearDirty(a);
    assert(!world.isDirty(a));
    world.setBlock(a, 1, 2, 3, 9);
    assert(world.chunkVersion(a) > v1);
    assert(world.getBlock(a, 1, 2, 3) == 9);

    SpatialHierarchy hierarchy;
    auto region = hierarchy.addRegion(0, 0, 0, 4);
    auto cluster = hierarchy.addChild(region, NodeType::Cluster);
    auto chunk = hierarchy.addChild(cluster, NodeType::Chunk);
    assert(hierarchy.parent(chunk) == cluster);
    assert(hierarchy.children(cluster).size() == 1);

    std::cout << "pvr_world_tests: PASS\n";
}
