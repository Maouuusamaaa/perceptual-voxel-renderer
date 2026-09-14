#pragma once
#include "minecraft/minecraft_world_adapter.hpp"
#include "minecraft/mcworld_reader.hpp"
#include "renderer/renderer_runtime.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
namespace pvr::viewer {
enum class RenderComparisonMode { Perceptual, FixedDistance };
struct ImportedRenderMetrics { RenderComparisonMode mode{}; std::size_t visible{}; std::size_t rejected{}; std::size_t drawCount{}; std::size_t importedChunks{}; std::size_t importedBlocks{}; std::uint64_t worldTruthVersion{}; std::string backend; std::string diagnostic; };
class ImportedWorldSession { public: bool load(const std::filesystem::path&); void setMode(RenderComparisonMode mode) noexcept{mode_=mode;} RenderComparisonMode mode()const noexcept{return mode_;} ImportedRenderMetrics render(const ViewFrustum&,std::size_t); std::size_t importedChunkCount()const noexcept{return data_.chunks.size();} std::size_t importedBlockCount()const noexcept{return importedBlocks_;} std::uint64_t worldSeed()const noexcept{return data_.metadata.seed;} std::uint64_t worldTruthVersion()const noexcept{return worldTruthVersion_;} const std::string& lastDiagnostic()const noexcept{return diagnostic_;} private: void rebuildCandidates(); RenderComparisonMode mode_{RenderComparisonMode::Perceptual}; minecraft::MinecraftWorldData data_; WorldTruth world_{0,16}; RendererRuntime runtime_; RendererInitResult rendererInit_{}; std::vector<PipelineCandidate> candidates_; std::size_t importedBlocks_{}; std::uint64_t worldTruthVersion_{}; std::string diagnostic_; bool loaded_{}; };
}
