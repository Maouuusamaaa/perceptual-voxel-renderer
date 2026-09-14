cmake_minimum_required(VERSION 3.20)
project(perceptual_voxel_renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_library(pvr_core
    engine/world/world_truth.cpp
    engine/spatial/spatial_hierarchy.cpp
    engine/representation/representation.cpp
    engine/mesh/greedy_mesher.cpp
    engine/mesh/mesh_cluster_builder.cpp
engine/mesh/mesh_cluster_culler.cpp
    engine/streaming/streaming_manager.cpp
    engine/core/job_system.cpp
    engine/gpu/gpu_scene.cpp
    engine/renderer/renderer_capability.cpp
    engine/renderer/vulkan_backend.cpp
    engine/renderer/render_budget.cpp
    engine/renderer/perceptual_culling.cpp
    engine/renderer/gpu_pipeline.cpp
    engine/renderer/gpu_culling.cpp
    engine/renderer/hierarchical_visibility.cpp
    engine/renderer/temporal_visibility.cpp
    engine/renderer/perceptual_pipeline.cpp
    engine/renderer/renderer_runtime.cpp
    engine/minecraft/mcworld_archive.cpp
    engine/minecraft/mcworld_reader.cpp
    engine/minecraft/minecraft_diagnostics.cpp
    engine/minecraft/minecraft_material_map.cpp
    engine/minecraft/minecraft_world_adapter.cpp
    engine/viewer/imported_world_session.cpp
    engine/viewer/debug_overlay.cpp
)
target_include_directories(pvr_core PUBLIC engine)
add_executable(pvr_world_tests tests/world/world_tests.cpp)
target_link_libraries(pvr_world_tests PRIVATE pvr_core)
enable_testing()
add_test(NAME pvr_world_tests COMMAND pvr_world_tests)

add_executable(pvr_mesh_tests tests/mesh/mesh_tests.cpp)
target_link_libraries(pvr_mesh_tests PRIVATE pvr_core)

add_executable(pvr_streaming_tests tests/streaming/streaming_tests.cpp)
target_link_libraries(pvr_streaming_tests PRIVATE pvr_core)
add_test(NAME pvr_mesh_tests COMMAND pvr_mesh_tests)
add_test(NAME pvr_streaming_tests COMMAND pvr_streaming_tests)

add_executable(pvr_gpu_scene_tests tests/gpu/gpu_scene_tests.cpp)
target_link_libraries(pvr_gpu_scene_tests PRIVATE pvr_core)
add_test(NAME pvr_gpu_scene_tests COMMAND pvr_gpu_scene_tests)

add_executable(pvr_renderer_capability_tests tests/gpu/renderer_capability_tests.cpp)
target_link_libraries(pvr_renderer_capability_tests PRIVATE pvr_core)
add_test(NAME pvr_renderer_capability_tests COMMAND pvr_renderer_capability_tests)

add_executable(pvr_vulkan_backend_tests tests/gpu/vulkan_backend_tests.cpp)
target_link_libraries(pvr_vulkan_backend_tests PRIVATE pvr_core)
add_test(NAME pvr_vulkan_backend_tests COMMAND pvr_vulkan_backend_tests)

add_executable(pvr_gpu_pipeline_tests tests/gpu/gpu_pipeline_tests.cpp)
target_link_libraries(pvr_gpu_pipeline_tests PRIVATE pvr_core)
add_test(NAME pvr_gpu_pipeline_tests COMMAND pvr_gpu_pipeline_tests)

add_executable(pvr_perceptual_culling_tests tests/gpu/perceptual_culling_tests.cpp)
target_link_libraries(pvr_perceptual_culling_tests PRIVATE pvr_core)
add_test(NAME pvr_perceptual_culling_tests COMMAND pvr_perceptual_culling_tests)

add_executable(pvr_budget_tests tests/renderer/budget_tests.cpp)
target_link_libraries(pvr_budget_tests PRIVATE pvr_core)
add_test(NAME pvr_budget_tests COMMAND pvr_budget_tests)

if(UNIX AND NOT APPLE)
  target_link_libraries(pvr_core PRIVATE dl)
endif()

add_executable(pvr_perceptual_benchmark benchmarks/perceptual_benchmark.cpp)
target_link_libraries(pvr_perceptual_benchmark PRIVATE pvr_core)

add_executable(pvr_gpu_culling_tests tests/gpu/gpu_culling_tests.cpp)
target_link_libraries(pvr_gpu_culling_tests PRIVATE pvr_core)
add_test(NAME pvr_gpu_culling_tests COMMAND pvr_gpu_culling_tests)

add_executable(pvr_gpu_draw_tests tests/gpu/gpu_draw_tests.cpp)
target_link_libraries(pvr_gpu_draw_tests PRIVATE pvr_core)
add_test(NAME pvr_gpu_draw_tests COMMAND pvr_gpu_draw_tests)

add_executable(pvr_hierarchical_visibility_tests tests/gpu/hierarchical_visibility_tests.cpp)
target_link_libraries(pvr_hierarchical_visibility_tests PRIVATE pvr_core)
add_test(NAME pvr_hierarchical_visibility_tests COMMAND pvr_hierarchical_visibility_tests)

add_executable(pvr_temporal_visibility_tests tests/gpu/temporal_visibility_tests.cpp)
target_link_libraries(pvr_temporal_visibility_tests PRIVATE pvr_core)
add_test(NAME pvr_temporal_visibility_tests COMMAND pvr_temporal_visibility_tests)

add_executable(pvr_perceptual_pipeline_tests tests/renderer/perceptual_pipeline_tests.cpp)
target_link_libraries(pvr_perceptual_pipeline_tests PRIVATE pvr_core)
add_test(NAME pvr_perceptual_pipeline_tests COMMAND pvr_perceptual_pipeline_tests)

add_executable(pvr_renderer_runtime_tests tests/renderer/renderer_runtime_tests.cpp)
target_link_libraries(pvr_renderer_runtime_tests PRIVATE pvr_core)
add_test(NAME pvr_renderer_runtime_tests COMMAND pvr_renderer_runtime_tests)

find_package(ZLIB REQUIRED)
target_link_libraries(pvr_core PRIVATE ZLIB::ZLIB)
add_executable(pvr_minecraft_reader_tests tests/minecraft/mcworld_reader_tests.cpp)
target_link_libraries(pvr_minecraft_reader_tests PRIVATE pvr_core)
target_compile_definitions(pvr_minecraft_reader_tests PRIVATE PVR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME pvr_minecraft_reader_tests COMMAND pvr_minecraft_reader_tests)

add_executable(pvr_minecraft_world_adapter_tests tests/minecraft/minecraft_world_adapter_tests.cpp)
target_link_libraries(pvr_minecraft_world_adapter_tests PRIVATE pvr_core)
add_test(NAME pvr_minecraft_world_adapter_tests COMMAND pvr_minecraft_world_adapter_tests)

add_executable(pvr_minecraft_viewer_tests tests/viewer/imported_world_session_tests.cpp)
target_link_libraries(pvr_minecraft_viewer_tests PRIVATE pvr_core)
target_compile_definitions(pvr_minecraft_viewer_tests PRIVATE PVR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME pvr_minecraft_viewer_tests COMMAND pvr_minecraft_viewer_tests)
add_executable(pvr_minecraft_viewer tools/pvr_minecraft_viewer.cpp)
target_link_libraries(pvr_minecraft_viewer PRIVATE pvr_core)
add_executable(pvr_minecraft_render_benchmark benchmarks/minecraft_render_benchmark.cpp)
target_link_libraries(pvr_minecraft_render_benchmark PRIVATE pvr_core)
