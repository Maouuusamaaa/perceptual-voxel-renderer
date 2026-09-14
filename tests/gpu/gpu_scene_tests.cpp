#include "gpu/gpu_scene.hpp"
#include <cassert>
#include <iostream>

int main() {
    pvr::GPUScene scene;
    pvr::GPUInstance instance{};
    instance.node_id = 42;
    instance.geometry_id = 7;
    instance.material_id = 3;
    instance.lod = 1;
    instance.importance = 0.75f;

    const auto id = scene.upsert(instance);
    assert(id == 0);
    assert(scene.size() == 1);
    assert(scene.at(0).node_id == 42);
    assert(scene.at(0).geometry_id == 7);

    instance.importance = 0.95f;
    assert(scene.upsert(instance) == 0);
    assert(scene.size() == 1);
    assert(scene.at(0).importance == 0.95f);

    scene.clear();
    assert(scene.size() == 0);
    std::cout << "gpu_scene_tests: PASS\n";
}
