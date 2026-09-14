#pragma once
#include "minecraft/minecraft_diagnostics.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
namespace pvr::minecraft {
struct MinecraftArchiveEntry { std::string path; std::vector<std::uint8_t> data; };
class MinecraftArchiveReader { public: static bool read(const std::filesystem::path&, std::vector<MinecraftArchiveEntry>&, std::vector<MinecraftDiagnostic>&); };
}
