#include "renderer/perceptual_pipeline.hpp"
#include <algorithm>
namespace pvr {
void PerceptualPipeline::add(const PipelineCandidate& c){candidates_.push_back(c);}
void PerceptualPipeline::clear(){candidates_.clear();}
PipelineResult PerceptualPipeline::execute(const ViewFrustum& view,std::size_t max_draws){
  temporal_.begin_frame();
  VisibilityHierarchy hierarchy;
  for(const auto& c:candidates_) hierarchy.add({c.node_id,c.position,c.radius,c.importance});
  auto spatial=hierarchy.cull({view.position,view.forward,view.fov_degrees,view.aspect,view.near_plane,view.far_plane});
  std::vector<PipelineCandidate> selected;
  selected.reserve(spatial.visible.size());
  for(const auto& n:spatial.visible){
    auto it=std::find_if(candidates_.begin(),candidates_.end(),[&](const auto& c){return c.node_id==n.node_id;});
    if(it!=candidates_.end()) selected.push_back(*it);
  }
  std::stable_sort(selected.begin(),selected.end(),[](const auto&a,const auto&b){return (a.importance+a.screen_coverage)>(b.importance+b.screen_coverage);});
  if(max_draws && selected.size()>max_draws) selected.resize(max_draws);
  PipelineResult out;
  out.rejected=candidates_.size()-selected.size();
  out.visible.reserve(selected.size());
  for(const auto& c:selected){
    temporal_.observe(c.node_id,true);
    GPUVisibleCandidate v{}; v.node_id=c.node_id; v.geometry_id=c.geometry_id; v.material_id=c.material_id; v.position=c.position; v.radius=c.radius; v.screen_coverage=c.screen_coverage; v.importance=c.importance;
    v.lod = c.screen_coverage>0.35f || temporal_.confidence(c.node_id)==VisibilityConfidence::High ? 0 : (c.screen_coverage>0.08f?1:2);
    v.visual_score=c.screen_coverage+c.importance;
    out.visible.push_back(v);
  }
  for(const auto& c:candidates_){
    if(std::none_of(selected.begin(),selected.end(),[&](const auto&s){return s.node_id==c.node_id;})) temporal_.observe(c.node_id,false);
  }
  return out;
}
}
