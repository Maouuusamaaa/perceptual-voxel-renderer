#include "renderer/temporal_visibility.hpp"
namespace pvr {
void TemporalVisibilityCache::observe(std::uint64_t id,bool visible){
  auto& e=entries_[id]; e.last_frame=frame_;
  if(visible){++e.visible_streak; e.miss_streak=0;} else {++e.miss_streak; e.visible_streak=0;}
}
void TemporalVisibilityCache::begin_frame(){
  ++frame_;
  for(auto it=entries_.begin();it!=entries_.end();){
    if(frame_>it->second.last_frame && frame_-it->second.last_frame>ttl_frames_) it=entries_.erase(it); else ++it;
  }
}
VisibilityConfidence TemporalVisibilityCache::confidence(std::uint64_t id) const {
  auto it=entries_.find(id); if(it==entries_.end()) return VisibilityConfidence::Low;
  const auto&e=it->second;
  if(e.visible_streak>=2) return VisibilityConfidence::High;
  if(e.visible_streak>=1 || e.miss_streak==1) return VisibilityConfidence::Medium;
  return VisibilityConfidence::Low;
}
bool TemporalVisibilityCache::contains(std::uint64_t id) const{return entries_.find(id)!=entries_.end();}
}
