#include "renderer/perceptual_culling.hpp"
#include <algorithm>
#include <cmath>
namespace pvr {
namespace {
float dot(Vec3 a, Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float length(Vec3 a){return std::sqrt(dot(a,a));}
}
CullingResult PerceptualCulling::evaluate(const std::vector<PerceptualCandidate>& in, const ViewFrustum& view, std::size_t budget) const {
    struct Ranked { VisibleCandidate c; float score; } ranked;
    std::vector<Ranked> candidates;
    candidates.reserve(in.size());
    const float half_fov = view.fov_degrees * 0.5f * 3.14159265f / 180.0f;
    const float cos_half = std::cos(half_fov);
    CullingResult out;
    for (const auto& c : in) {
        Vec3 d{c.position.x-view.position.x,c.position.y-view.position.y,c.position.z-view.position.z};
        const float dist=length(d);
        if (dist < view.near_plane || dist-c.radius > view.far_plane || dist <= 0.0001f) { ++out.rejected; continue; }
        const float facing=dot(d,view.forward)/dist;
        if (facing < cos_half && !c.force_full) { ++out.rejected; continue; }
        const float score = c.importance*2.0f + c.screen_coverage*4.0f + facing - dist*0.01f;
        VisibleCandidate vc{};
        static_cast<PerceptualCandidate&>(vc)=c;
        vc.visual_score=score;
        if (c.force_full || c.screen_coverage > 0.35f || c.importance > 0.8f) vc.lod=0;
        else if (c.screen_coverage > 0.08f) vc.lod=1;
        else vc.lod=2;
        candidates.push_back({vc,score});
    }
    const std::size_t count=std::min(budget,candidates.size());
    if (count < candidates.size()) {
        std::nth_element(candidates.begin(), candidates.begin()+count, candidates.end(), [](const Ranked&a,const Ranked&b){return a.score>b.score;});
    }
    std::sort(candidates.begin(), candidates.begin()+count, [](const Ranked&a,const Ranked&b){return a.score>b.score;});
    out.visible.reserve(count);
    for(std::size_t i=0;i<count;++i) out.visible.push_back(candidates[i].c);
    out.rejected += candidates.size()-count;
    return out;
}
}
