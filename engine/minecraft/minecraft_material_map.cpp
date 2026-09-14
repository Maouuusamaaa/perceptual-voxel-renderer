#include "minecraft/minecraft_material_map.hpp"
namespace pvr::minecraft {
MinecraftMaterialMap::MinecraftMaterialMap(MaterialID fallback):fallback_(fallback){}
MaterialMapResult MinecraftMaterialMap::map(std::uint32_t id) const { if(id<=65534) return {static_cast<MaterialID>(id),false}; return {fallback_,true}; }
}
