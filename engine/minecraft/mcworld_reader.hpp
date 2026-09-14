#pragma once
#include "minecraft/minecraft_diagnostics.hpp"
#include "minecraft/minecraft_types.hpp"
#include <filesystem>
namespace pvr::minecraft { struct MinecraftReadResult { MinecraftWorldData data; std::vector<MinecraftDiagnostic> diagnostics; bool ok() const noexcept; }; class MinecraftWorldReader { public: static MinecraftReadResult read(const std::filesystem::path&); }; }
