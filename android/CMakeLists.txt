cmake_minimum_required(VERSION 3.20)
project(pvr_android LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_library(pvr_android SHARED
    ../engine/world/world_truth.cpp
    ../engine/spatial/spatial_hierarchy.cpp
    ../engine/representation/representation.cpp
    ../engine/mesh/greedy_mesher.cpp
    ../engine/streaming/streaming_manager.cpp
    ../engine/core/job_system.cpp
    ../engine/gpu/gpu_scene.cpp
    ../engine/renderer/renderer_capability.cpp
    ../engine/renderer/vulkan_backend.cpp
    ../engine/renderer/gpu_pipeline.cpp
    ../engine/renderer/perceptual_culling.cpp
    ../engine/renderer/render_budget.cpp
    ../engine/renderer/hierarchical_visibility.cpp
    ../engine/renderer/temporal_visibility.cpp
    ../engine/renderer/perceptual_pipeline.cpp
    ../engine/renderer/renderer_runtime.cpp
    ../engine/minecraft/mcworld_archive.cpp
    ../engine/minecraft/mcworld_reader.cpp
    ../engine/minecraft/minecraft_diagnostics.cpp
    ../engine/minecraft/minecraft_material_map.cpp
    ../engine/minecraft/minecraft_world_adapter.cpp
    app/src/main/cpp/minecraft_android_bridge.cpp)
target_include_directories(pvr_android PRIVATE ../engine)
find_library(log-lib log)
find_library(android-lib android)
find_library(z-lib z)
target_link_libraries(pvr_android ${log-lib} ${android-lib} ${z-lib})
