#include "renderer/hierarchical_visibility.hpp"
#include <cassert>
using namespace pvr;
int main(){
  VisibilityHierarchy h;
  h.add({1,{0,0,0},2,10});
  h.add({2,{100,0,0},2,10});
  h.add({3,{0,0,20},2,1});
  VisibilityQuery q{{0,0,0},{0,0,1},90.0f,1.0f,0.1f,50.0f};
  auto r=h.cull(q);
  assert(r.visible.size()==2);
  assert(r.rejected==1);
  assert(r.visible[0].node_id==1);
  return 0;
}
