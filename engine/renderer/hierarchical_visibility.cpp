#include "renderer/hierarchical_visibility.hpp"
#include <algorithm>
#include <cmath>
namespace pvr {
namespace {
float dot(Vec3 a, Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float len(Vec3 a){return std::sqrt(dot(a,a));}
Vec3 norm(Vec3 a){float l=len(a); return l>1e-6f?Vec3{a.x/l,a.y/l,a.z/l}:Vec3{0,0,1};}
}
void VisibilityHierarchy::add(const VisibilityNode& n){nodes_.push_back(n);}
void VisibilityHierarchy::clear(){nodes_.clear();}
VisibilityResult VisibilityHierarchy::cull(const VisibilityQuery& q) const {
  VisibilityResult out;
  const Vec3 f=norm(q.forward);
  const float half=std::clamp(q.fov_degrees,1.0f,179.0f)*0.5f*3.14159265359f/180.0f;
  const float cos_half=std::cos(half);
  for(const auto& n:nodes_){
    Vec3 d{n.center.x-q.position.x,n.center.y-q.position.y,n.center.z-q.position.z};
    float dist=len(d);
    bool ok=dist+n.radius>=q.near_plane && dist-n.radius<=q.far_plane;
    if(ok && dist>1e-5f) ok=dot(norm(d),f)>=cos_half-std::min(0.5f,n.radius/std::max(dist,1.0f));
    if(ok) out.visible.push_back(n); else ++out.rejected;
  }
  std::stable_sort(out.visible.begin(),out.visible.end(),[](const auto&a,const auto&b){return a.importance>b.importance;});
  return out;
}
}
