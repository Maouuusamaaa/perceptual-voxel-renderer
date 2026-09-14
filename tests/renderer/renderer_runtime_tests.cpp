#include "renderer/renderer_runtime.hpp"
#include <cassert>
using namespace pvr;


static void cycle42_48_runtime_to_indirect_buffer_contract() {
  pvr::RendererRuntime runtime;

  pvr::PipelineCandidate candidate{
      501,
      {0,0,5},
      1.0f,
      0.9f,
      5.0f,
      0,
      17
  };

  runtime.add_candidate(candidate);

  auto result = runtime.render(
      {{0,0,0},{0,0,1},90,1,0.1f,100},
      1
  );

  assert(result.draw_count == 1);
  assert(result.indirect_commands.size() == 1);

  std::vector<pvr::MeshCluster> clusters(1);
  clusters[0].first_index = 0;
  clusters[0].index_count = 36;
  clusters[0].first_vertex = 0;
  clusters[0].vertex_count = 24;
  clusters[0].material = 17;

  /*
   * Cycle #42.48:
   * RendererRuntime must expose the generated frame as a validated
   * IndirectBuffer using the supplied MeshCluster geometry source.
   */
  auto buffer = runtime.build_indirect_buffer(result, clusters);

  assert(buffer.has_value());
  assert(buffer->size() == 1);
  assert(buffer->validate(36));

  const auto& command = (*buffer)[0];

  assert(command.index_count == 36);
  assert(command.instance_count == 1);
  assert(command.first_index == 0);
  assert(command.vertex_offset == 0);
  assert(command.first_instance == 0);
}

static void cycle42_35_indirect_command_metadata_contract() {
  pvr::RendererRuntime runtime;

  pvr::PipelineCandidate candidate{
      77,
      {0,0,5},
      1.0f,
      0.9f,
      5.0f,
      123
  };

  runtime.add_candidate(candidate);

  auto result = runtime.render(
      {{0,0,0},{0,0,1},90,1,0.1f,100},
      1
  );

  assert(result.draw_count == 1);
  assert(result.indirect_commands.size() == 1);

  /*
   * The renderer runtime must preserve the candidate geometry identity
   * when producing its indirect GPU command.
   */
  assert(result.indirect_commands[0].node_id == 77);
  assert(result.indirect_commands[0].geometry_id == 123);
}


static void cycle42_36_indirect_command_material_contract() {
  pvr::RendererRuntime runtime;

  pvr::PipelineCandidate candidate{
      88,
      {0,0,5},
      1.0f,
      0.9f,
      5.0f,
      321,
      17
  };

  runtime.add_candidate(candidate);

  auto result = runtime.render(
      {{0,0,0},{0,0,1},90,1,0.1f,100},
      1
  );

  assert(result.draw_count == 1);
  assert(result.indirect_commands.size() == 1);

  /*
   * Material identity must survive the perceptual pipeline and reach
   * the generated GPU indirect command.
   */
  assert(result.indirect_commands[0].node_id == 88);
  assert(result.indirect_commands[0].geometry_id == 321);
  assert(result.indirect_commands[0].material_id == 17);
}

int main(){
  cycle42_48_runtime_to_indirect_buffer_contract();
  cycle42_36_indirect_command_material_contract();
  cycle42_35_indirect_command_metadata_contract();
  RendererRuntime runtime;
  auto init=runtime.initialize();
  assert(init.mode==RenderMode::Vulkan || init.mode==RenderMode::CPUFallback);
  PipelineCandidate a{1,{0,0,5},1,.8f,5,10};
  PipelineCandidate b{2,{0,0,80},1,.02f,.1f,11};
  runtime.add_candidate(a); runtime.add_candidate(b);
  auto r=runtime.render({{0,0,0},{0,0,1},90,1,.1f,100},1);
  assert(r.draw_count==1);
  assert(r.visible_count==1);
  assert(r.indirect_commands.size()==1);
  assert(r.indirect_commands[0].node_id==1);
  return 0;
}
