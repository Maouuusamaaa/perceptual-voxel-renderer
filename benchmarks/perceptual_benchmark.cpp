#include "renderer/perceptual_culling.hpp"
#include <chrono>
#include <iostream>
#include <vector>
int main(){
    std::vector<pvr::PerceptualCandidate> scene;
    scene.reserve(100000);
    for(int i=0;i<100000;++i){
        scene.push_back({{float((i%200)-100), float((i/200)%50), float((i/10000)*20+1)},1.0f,float((i%100)+1)/100.0f,(i%97==0)?1.0f:0.1f,false});
    }
    pvr::PerceptualCulling culling;
    pvr::ViewFrustum view{{0,0,0},{0,0,1},90,16.0f/9.0f,0.1f,500};
    const auto start=std::chrono::steady_clock::now();
    auto result=culling.evaluate(scene,view,5000);
    const auto end=std::chrono::steady_clock::now();
    const double ms=std::chrono::duration<double,std::milli>(end-start).count();
    std::cout << "perceptual_benchmark\n";
    std::cout << "candidates=" << scene.size() << " visible=" << result.visible.size() << " rejected=" << result.rejected << "\n";
    std::cout << "cpu_culling_ms=" << ms << "\n";
    return 0;
}
