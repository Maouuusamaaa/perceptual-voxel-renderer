#include "minecraft/minecraft_world_adapter.hpp"
#include "minecraft/minecraft_material_map.hpp"
namespace pvr::minecraft {
MinecraftImportResult MinecraftWorldAdapter::import(const MinecraftWorldData& data, pvr::WorldTruth& world, const MinecraftImportOptions& options) {
    MinecraftImportResult result{true,{}}; MinecraftMaterialMap map(options.fallbackMaterial);
    if(data.metadata.sectionSize == 0) { result.ok=false; result.report.diagnostics.push_back({MinecraftDiagnosticSeverity::Error,"metadata","section size is zero",false}); return result; }
    for(const auto& chunk:data.chunks) for(const auto& section:chunk.sections) {
        const pvr::ChunkId id{chunk.x, section.y, chunk.z};
        for(const auto& block:section.blocks) {
            auto mapped=map.map(block.sourceStateId);
            if(mapped.fallback){++result.report.fallbackMaterials;result.report.diagnostics.push_back({MinecraftDiagnosticSeverity::Warning,"block_state:"+std::to_string(block.sourceStateId),"source block state is outside renderer material range; fallback material used",true});}
            world.setBlock(id,block.x,block.y,block.z,mapped.material); ++result.report.importedBlocks;
        }
    }
    return result;
}
}
