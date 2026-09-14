# Minecraft Bedrock World Compatibility + PVR Viewer — Design Specification

Date: 2026-09-12
Status: Design approved direction; implementation not started

## 1. Goal

Extend the Perceptual Voxel Renderer (PVR) with a Minecraft Bedrock compatibility path whose first input is an exported `.mcworld` world. The first milestone is not replacing Minecraft Bedrock's internal RenderDragon renderer. Instead, PVR will ingest compatible world data and render it through the existing perceptual rendering architecture.

Success means a reproducible pipeline can take a supported `.mcworld`, extract its world data, convert it into PVR World Truth, stream and represent terrain at multiple fidelity levels, and display/benchmark the result. The system must preserve Minecraft world data semantics where supported and clearly report unsupported data instead of silently corrupting it.

## 2. Scope and non-goals

In scope:
- `.mcworld` as the first controlled input format.
- Import/extraction layer separated from PVR core.
- Deterministic conversion into World Truth.
- Chunk/section streaming into the existing representation hierarchy.
- Minecraft-compatible block/material identity mapping where source data is available.
- Unsupported-feature reporting.
- Viewer and benchmark integration.
- Android-ready architecture, without claiming direct integration into the official Minecraft client.

Out of scope for the first milestone:
- Replacing or hooking Minecraft Bedrock's RenderDragon renderer.
- Modifying the official Minecraft APK/client.
- Full compatibility with every Bedrock block entity, command, script, animation, particle, or gameplay system.
- Perfect reproduction of Minecraft lighting/redstone/gameplay.
- Reimplementing Minecraft as a complete client.

## 3. Architecture

Minecraft `.mcworld`
  -> archive/world extractor
  -> Bedrock world-data reader
  -> compatibility mapper
  -> PVR World Truth
  -> Representation Manager
  -> Streaming Manager
  -> Greedy Mesher / surface extraction
  -> GPUScene
  -> Hierarchical Visibility
  -> Temporal Visibility
  -> Perceptual Budget + LOD
  -> GPU Culling / indirect draw contract
  -> Vulkan backend or CPU fallback
  -> Viewer

The compatibility layer is an adapter. It must not leak Minecraft-specific parsing details into renderer code.

## 4. Input and data model

The importer accepts a `.mcworld` archive as a read-only source. The importer extracts only the data required by the current milestone and produces normalized records for PVR.

The normalized representation should contain, at minimum:
- dimension identifier
- world/chunk coordinates
- vertical section coordinates
- block state/material identity
- section presence/emptiness
- world metadata needed for coordinate interpretation
- optional block/entity metadata only when explicitly supported

PVR World Truth remains the renderer's source of truth after import. Representation caches are derived and disposable.

## 5. Compatibility strategy

Compatibility is capability-based rather than binary. The importer reports support by feature category, for example:
- terrain/block states: supported when decoded
- block textures/materials: supported through an explicit mapping table
- block entities: unsupported until a dedicated adapter exists
- dynamic gameplay: not part of the viewer milestone
- lighting: reconstructed/approximated by PVR lighting, not assumed to equal the vanilla client

Unknown or unsupported records must generate diagnostics rather than being silently interpreted as a different block.

## 6. Coordinate and chunk handling

Minecraft coordinates are converted into deterministic PVR spatial IDs. The logical world graph remains separate from render hierarchy.

The PVR render hierarchy remains:
WORLD -> MACRO REGION -> CLUSTER -> CHUNK -> MESH/SURFACE.

The previously selected conceptual 16x16 Minecraft chunk model is treated as source-world semantics; internal PVR region/cluster dimensions remain benchmark parameters.

## 7. Material pipeline

The compatibility layer maps source block states to PVR MaterialID values. Material definitions remain renderer-facing and are stored in the existing GPU Material Table path.

The first implementation should prioritize a deterministic material fallback system over attempting full Minecraft visual parity. A missing texture/material must be visible in diagnostics and use an explicit fallback material.

## 8. Perceptual rendering behavior

Imported Minecraft terrain is rendered using the existing PVR principles:
- FULL representation near/high-value geometry
- REDUCED representation at intermediate importance
- COARSE terrain/landmark representation for distant terrain
- ABSTRACT horizon representation where appropriate
- screen-space coverage and visibility influence work allocation
- temporal prediction preloads likely future visibility
- hysteresis prevents representation thrashing

The primary experiment is comparison between conventional fixed render distance and PVR perceptual view distance using the same imported world.

## 9. Viewer milestone

The first viewer should provide:
- camera movement
- basic Minecraft terrain visualization
- chunk streaming status
- current representation statistics
- visible/rejected candidate counts
- CPU frame/culling timing
- renderer backend status (Vulkan or CPU fallback)
- import diagnostics

A debug mode should allow disabling perceptual LOD so a fixed-distance baseline can be compared against PVR.

## 10. Android path

Android is a target runtime, not a prerequisite for the first parser milestone. The existing Android native packaging path will be extended only after the desktop importer/viewer is testable.

The Android build must use the existing renderer abstraction and capability tiers. Vulkan availability is detected at runtime; CPU fallback remains available for development and diagnostic builds.

The official Minecraft Android client remains a separate application. The PVR viewer is not presented as an in-client renderer replacement.

## 11. Testing

Tests will be layered:
1. archive/import tests using small fixture worlds
2. chunk/section decoding tests
3. deterministic coordinate and block-state mapping tests
4. unsupported-feature diagnostics tests
5. World Truth equivalence tests for imported terrain
6. streaming/representation tests using imported chunks
7. rendering pipeline regression tests
8. benchmark comparison: fixed-distance vs perceptual view distance
9. Android arm64 build/runtime tests when an Android SDK/NDK environment is available

Fixtures should be minimal and legally/technically appropriate for testing. The test harness must not depend on a user's private Minecraft world being present.

## 12. Error handling

The importer must fail closed for malformed or unsupported critical data. Recoverable unsupported features should be recorded in structured diagnostics. Every conversion stage should expose enough information to identify the source dimension/chunk/section associated with an error.

No renderer cache may become the authoritative source of imported world data.

## 13. Milestones

M1 — `.mcworld` ingestion and normalized terrain records.
M2 — normalized records -> PVR World Truth.
M3 — imported terrain -> streaming/representation/mesh pipeline.
M4 — viewer with perceptual/fixed-distance comparison.
M5 — Android arm64 build and device validation.
M6 — expanded Bedrock feature compatibility based on measured demand.

## 14. Acceptance criteria for the first implementation cycle

The cycle is accepted when:
- a supported `.mcworld` fixture can be ingested deterministically;
- terrain data reaches PVR World Truth without renderer-specific coupling;
- imported chunks can pass through existing streaming and representation systems;
- the viewer can render the imported terrain through the CPU fallback and, when a capable environment is available, the Vulkan backend;
- unsupported data is reported explicitly;
- fixed-distance and perceptual modes produce comparable benchmark metrics;
- all existing PVR tests remain passing.

## 15. Known constraints

The current development container has no usable Vulkan physical device/ICD and no Android SDK/NDK/Gradle toolchain, so desktop Vulkan execution and Android packaging require environment-dependent validation later. These are validation constraints, not reasons to couple the compatibility layer to the official Minecraft renderer.
