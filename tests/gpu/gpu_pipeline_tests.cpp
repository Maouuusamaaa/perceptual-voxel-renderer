#include "renderer/gpu_pipeline.hpp"
#include <cassert>
#include <iostream>

int main() {
    pvr::GPUCommandBuilder builder;
    pvr::GPUInstance a{1, 10, 2, 0, 1.0f};
    pvr::GPUInstance b{2, 11, 3, 1, 0.5f};
    builder.add(a, true);
    builder.add(b, false);
    const auto commands = builder.build();
    assert(commands.size() == 1);
    assert(commands[0].node_id == 1);
    assert(commands[0].instance_count == 1);
    assert(commands[0].lod == 0);
    std::cout << "gpu_pipeline_tests: PASS\n";
}
