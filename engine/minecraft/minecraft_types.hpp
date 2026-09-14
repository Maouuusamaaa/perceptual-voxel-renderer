#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace pvr::minecraft {
struct MinecraftWorldMetadata { std::string worldName; std::uint64_t seed{}; std::uint32_t sectionSize{}; std::vector<std::string> dimensions; bool operator==(const MinecraftWorldMetadata&) const = default; };
struct MinecraftBlockStateRecord { std::uint32_t x{}, y{}, z{}; std::uint32_t sourceStateId{}; bool operator==(const MinecraftBlockStateRecord&) const = default; };
struct MinecraftSectionRecord { std::int32_t y{}; std::vector<MinecraftBlockStateRecord> blocks; bool operator==(const MinecraftSectionRecord&) const = default; };
struct MinecraftChunkRecord { std::int64_t x{}, z{}; std::string dimension; std::vector<MinecraftSectionRecord> sections; bool operator==(const MinecraftChunkRecord&) const = default; };
struct MinecraftWorldData { MinecraftWorldMetadata metadata; std::vector<MinecraftChunkRecord> chunks; bool operator==(const MinecraftWorldData&) const = default; };
}
