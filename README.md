# Perceptual Voxel Renderer

Experimental voxel renderer prototype based on perceptual rendering: render work is prioritized by visual value rather than a fixed render distance.

Architecture: World Truth -> Representation Cache -> GPU Scene -> Perceptual Culling -> GPU command preparation -> renderer backend.

Current status:
- World Truth, spatial hierarchy, representations, streaming, greedy meshing: implemented.
- GPU scene, capability tiers and Vulkan loader/instance bootstrap: implemented.
- Perceptual culling, LOD selection and adaptive render budget: implemented and tested.
- Indirect-command preparation: implemented as a renderer-neutral abstraction.
- Actual Vulkan logical-device/swapchain/shader submission remains environment-dependent and is not claimed as verified here.
- Android native CMake entry point is prepared; APK packaging requires Android SDK/NDK.

Build:

```sh
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Benchmark executable: `pvr_perceptual_benchmark`.
