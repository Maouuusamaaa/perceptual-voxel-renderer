#include "viewer/debug_overlay.hpp"
#include <iostream>
int main(int argc,char**argv){if(argc!=2){std::cerr<<"usage: pvr_minecraft_viewer <world.mcworld>\n";return 2;}pvr::viewer::ImportedWorldSession s;if(!s.load(argv[1])){std::cerr<<"import failed: "<<s.lastDiagnostic()<<"\n";return 1;}pvr::ViewFrustum v{{0,0,0},{0,0,1},90,1,0.1f,100};for(auto mode:{pvr::viewer::RenderComparisonMode::FixedDistance,pvr::viewer::RenderComparisonMode::Perceptual}){s.setMode(mode);std::cout<<pvr::viewer::formatDebugOverlay(s.render(v,5000))<<"\n";}return 0;}
