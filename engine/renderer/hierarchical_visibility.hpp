#pragma once
#include "renderer/perceptual_culling.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace pvr {
struct VisibilityNode { std::uint64_t node_id{}; Vec3 center{}; float radius{1.0f}; float importance{}; };
struct VisibilityQuery { Vec3 position{}; Vec3 forward{0,0,1}; float fov_degrees{90}; float aspect{1}; float near_plane{0.1f}; float far_plane{100}; };
struct VisibilityResult { std::vector<VisibilityNode> visible; std::size_t rejected{}; };
class VisibilityHierarchy {
public:
    void add(const VisibilityNode& node);
    void clear();
    VisibilityResult cull(const VisibilityQuery& query) const;
private:
    std::vector<VisibilityNode> nodes_;
};
}
