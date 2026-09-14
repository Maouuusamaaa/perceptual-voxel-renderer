#include "renderer/temporal_visibility.hpp"
#include <cassert>
using namespace pvr;
int main(){
  TemporalVisibilityCache c(2);
  c.observe(7,true); assert(c.confidence(7)==VisibilityConfidence::Medium); c.observe(7,true); assert(c.confidence(7)==VisibilityConfidence::High);
  c.observe(7,false); assert(c.confidence(7)==VisibilityConfidence::Medium);
  c.observe(7,false); assert(c.confidence(7)==VisibilityConfidence::Low);
  c.observe(8,true); c.begin_frame(); c.begin_frame(); c.begin_frame();
  assert(!c.contains(8));
  return 0;
}
