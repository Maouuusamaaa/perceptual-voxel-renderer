#include "renderer/renderer_runtime.hpp"
namespace pvr {
RendererInitResult RendererRuntime::initialize(){
  auto r=vulkan_.initialize();
  if(r.ok){mode_=RenderMode::Vulkan; return {mode_,true,"Vulkan physical-device path available."};}
  mode_=RenderMode::CPUFallback;
  return {mode_,false,"CPU fallback active: "+r.message};
}
void RendererRuntime::add_candidate(const PipelineCandidate& c){pipeline_.add(c);}
RenderFrameResult RendererRuntime::render(const ViewFrustum& view,std::size_t max_draws){
  auto p=pipeline_.execute(view,max_draws);
  RenderFrameResult out; out.visible_count=p.visible.size(); out.draw_count=p.visible.size(); out.rejected=p.rejected;
  out.indirect_commands.reserve(p.visible.size());
  for(const auto& v:p.visible) out.indirect_commands.push_back({v.node_id,v.geometry_id,v.material_id,static_cast<std::uint32_t>(v.lod),1});
  return out;
}


std::optional<IndirectBuffer>
RendererRuntime::build_indirect_buffer(
    const RenderFrameResult& frame,
    const std::vector<MeshCluster>& clusters
) {
    GPUCommandBuilder builder;

    for (const auto& command : frame.indirect_commands) {
        GPUInstance instance{
            command.node_id,
            command.geometry_id,
            command.material_id,
            command.lod,
            1.0f
        };

        builder.add(instance, true);
    }

    return builder.build_indirect_buffer(clusters);
}

}
