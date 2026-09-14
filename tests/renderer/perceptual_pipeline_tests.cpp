#include "renderer/perceptual_pipeline.hpp"
#include <cassert>
#include <cstdio>
using namespace pvr;
int main(){
  PerceptualPipeline p(2);
  p.add({1,{0,0,5},1,0.5f,10});
  p.add({2,{0,0,100},1,0.01f,1});
  ViewFrustum v{{0,0,0},{0,0,1},90.0f,1.0f,0.1f,150.0f};
  auto r=p.execute(v,1); fprintf(stderr,"visible=%zu rejected=%zu\n",r.visible.size(),r.rejected); for(auto n:r.visible) fprintf(stderr,"id=%llu\n",(unsigned long long)n.node_id);
  assert(r.visible.size()==1);
  assert(r.visible[0].node_id==1);
  assert(r.rejected==1);
  return 0;
}
