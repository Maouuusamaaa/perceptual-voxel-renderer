#pragma once
#include <cstddef>
#include <vector>
namespace pvr {
struct Vec3 { float x{}, y{}, z{}; };
struct ViewFrustum { Vec3 position; Vec3 forward; float fov_degrees{90}; float aspect{1}; float near_plane{0.1f}; float far_plane{100}; };
struct PerceptualCandidate { Vec3 position; float radius{1}; float screen_coverage{1}; float importance{0}; bool force_full{false}; };
struct VisibleCandidate : PerceptualCandidate { int lod{0}; float visual_score{0}; };
struct CullingResult { std::vector<VisibleCandidate> visible; std::size_t rejected{0}; };
class PerceptualCulling {
public:
    CullingResult evaluate(const std::vector<PerceptualCandidate>& candidates, const ViewFrustum& view, std::size_t budget) const;
};
}
