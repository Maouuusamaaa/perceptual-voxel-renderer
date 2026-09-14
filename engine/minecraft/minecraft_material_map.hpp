#pragma once
#include "minecraft/minecraft_diagnostics.hpp"
#include <cstdint>
#include <vector>
namespace pvr::minecraft {
using MaterialID = std::uint16_t;
struct MaterialMapResult { MaterialID material{}; bool fallback{}; };
class MinecraftMaterialMap { public: explicit MinecraftMaterialMap(MaterialID fallback); MaterialMapResult map(std::uint32_t sourceStateId) const; private: MaterialID fallback_; };
}
