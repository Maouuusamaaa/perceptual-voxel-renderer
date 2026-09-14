#include "renderer/perceptual_culling.hpp"
#include <cassert>
#include <iostream>

int main() {
    pvr::PerceptualCulling culling;
    pvr::ViewFrustum view{{0,0,0}, {1,0,0}, 90.0f, 1.0f, 0.1f, 100.0f};
    pvr::PerceptualCandidate near{{1,0,0}, 1.0f, 1.0f, 0.9f, false};
    pvr::PerceptualCandidate behind{{-10,0,0}, 1.0f, 1.0f, 0.9f, false};
    auto result = culling.evaluate({near, behind}, view, 1);
    assert(result.visible.size() == 1);
    assert(result.visible[0].position.x == 1.0f);
    assert(result.visible[0].lod == 0);
    std::cout << "perceptual_culling_tests: PASS\n";
}
