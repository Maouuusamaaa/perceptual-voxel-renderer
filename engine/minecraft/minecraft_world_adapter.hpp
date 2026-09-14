#pragma once
#include "minecraft/minecraft_diagnostics.hpp"
#include "minecraft/minecraft_types.hpp"
#include "world/world_truth.hpp"
#include <cstdint>
#include <vector>
namespace pvr::minecraft {
struct MinecraftImportOptions { std::uint16_t fallbackMaterial{65535}; };
struct MinecraftCompatibilityReport { std::uint64_t importedBlocks{}; std::uint64_t fallbackMaterials{}; std::vector<MinecraftDiagnostic> diagnostics; };
struct MinecraftImportResult { bool ok{}; MinecraftCompatibilityReport report; };
class MinecraftWorldAdapter { public: static MinecraftImportResult import(const MinecraftWorldData&, pvr::WorldTruth&, const MinecraftImportOptions&); };
}
