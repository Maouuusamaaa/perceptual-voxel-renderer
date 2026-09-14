#include "viewer/imported_world_session.hpp"
#include <chrono>
#include <iostream>
int main(int argc,char**argv){if(argc!=2)return 2;pvr::viewer::ImportedWorldSession s;if(!s.load(argv[1]))return 1;pvr::ViewFrustum v{{0,0,0},{0,0,1},90,1,0.1f,100};for(auto mode:{pvr::viewer::RenderComparisonMode::FixedDistance,pvr::viewer::RenderComparisonMode::Perceptual}){s.setMode(mode);auto t=std::chrono::steady_clock::now();auto r=s.render(v,5000);auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();std::cout<<(mode==pvr::viewer::RenderComparisonMode::FixedDistance?"fixed":"perceptual")<<" visible="<<r.visible<<" rejected="<<r.rejected<<" render_ms="<<ms<<"\n";}}
