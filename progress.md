# Plan: docs/plans/2026-09-12-minecraft-bedrock-world-compatibility.md
# Perceptual Voxel Renderer — Progress

## Completed

- Task 1: Engine core + immutable World Truth
- Task 2: Representation + streaming + greedy meshing + async job foundation
- Task 3 foundation: GPUScene + renderer capability tiers + Vulkan loader/instance bootstrap
- Phase 3C: renderer-neutral GPU culling/indirect-draw contracts + GLSL shader prototypes
- Phase 3D logical pipeline: hierarchical visibility, temporal visibility cache, perceptual pipeline integration, runtime fallback facade
- Task 4: adaptive render budget + CPU benchmark + Android native CMake/Gradle packaging path

## Current architecture

World Truth → Representation Cache → Streaming → Greedy Mesh → GPUScene →
Hierarchical Visibility → Temporal Visibility → Perceptual Budget/LOD →
GPU Culling Contract → Indirect Draw Contract → Vulkan Backend / CPU fallback

## Verification

Fresh CMake build and CTest: 15/15 tests passed.

Android Gradle APK packaging is configuration-complete but not locally executable in this container because Android SDK/NDK and Gradle are not installed.

Actual Vulkan GPU execution is runtime-gated: this container has libvulkan.so.1 but no usable Vulkan physical device/ICD and no Vulkan shader compiler. The code therefore does not claim verified GPU dispatch, swapchain presentation, Hi-Z execution, or indirect vkCmdDrawIndexedIndirect execution.

## Next environment-dependent validation

1. Build on a Vulkan-capable Linux machine with Vulkan development headers/compiler and a working ICD.
2. Build Android arm64-v8a with Android SDK + NDK.
3. Run the same test suite on-device.
4. Capture GPU timings and compare fixed render-distance versus perceptual view distance.

- Task 1: complete
- Task 2: complete
- Task 3: complete
