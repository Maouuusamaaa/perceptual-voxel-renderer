#include "renderer/gpu_culling.hpp"
#include <cassert>
#include <iostream>

int main() {
    pvr::GPUCullingFrame frame;
    frame.max_draws = 2;
    frame.candidates = {
        {1, {1,0,0}, 1.0f, 1.0f, 0.9f, 0, 10},
        {2, {2,0,0}, 1.0f, 0.1f, 0.1f, 0, 11},
        {3, {-5,0,0}, 1.0f, 1.0f, 0.9f, 0, 12}
    };
    pvr::ViewFrustum view{{0,0,0},{1,0,0},90,1,0.1f,100};
    pvr::GPUCullingPipeline pipeline;
    auto out = pipeline.execute(frame, view);
    assert(out.visible.size() == 2);
    assert(out.visible[0].node_id == 1);
    assert(out.indirect_commands.size() == 2);
    assert(out.rejected == 1);
    assert(out.indirect_commands[0].instance_count == 1);
    assert(out.dispatch.groups_x == 1);
    std::cout << "gpu_culling_tests: PASS\n";
}
