#include "world/world_truth.hpp"
#include <functional>
#include <stdexcept>

namespace pvr {
std::string ChunkId::toString() const { return std::to_string(x)+","+std::to_string(y)+","+std::to_string(z); }
std::size_t ChunkIdHash::operator()(const ChunkId& id) const noexcept {
    auto h = std::hash<std::int64_t>{}(id.x);
    h ^= std::hash<std::int64_t>{}(id.y) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
    h ^= std::hash<std::int64_t>{}(id.z) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
    return h;
}
WorldTruth::WorldTruth(std::uint64_t seed, std::uint32_t sectionSize): seed_(seed), sectionSize_(sectionSize) {
    if (sectionSize_ == 0) throw std::invalid_argument("section size must be non-zero");
}
std::uint64_t WorldTruth::seed() const noexcept { return seed_; }
std::uint64_t WorldTruth::key(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept {
    return (std::uint64_t{x}<<42) ^ (std::uint64_t{y}<<21) ^ std::uint64_t{z};
}
void WorldTruth::setBlock(ChunkId id, std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint16_t value) {
    if (x >= sectionSize_ || y >= sectionSize_ || z >= sectionSize_) throw std::out_of_range("block coordinate");
    auto& c = chunks_[id];
    c.blocks[key(x,y,z)] = value;
    c.dirty = true;
    ++c.version;
}
std::uint16_t WorldTruth::getBlock(ChunkId id, std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
    if (x >= sectionSize_ || y >= sectionSize_ || z >= sectionSize_) throw std::out_of_range("block coordinate");
    auto it = chunks_.find(id); if (it == chunks_.end()) return 0;
    auto b = it->second.blocks.find(key(x,y,z)); return b == it->second.blocks.end() ? 0 : b->second;
}
bool WorldTruth::isDirty(ChunkId id) const { auto it=chunks_.find(id); return it!=chunks_.end() && it->second.dirty; }
void WorldTruth::clearDirty(ChunkId id) { auto it=chunks_.find(id); if (it!=chunks_.end()) it->second.dirty=false; }
std::uint64_t WorldTruth::chunkVersion(ChunkId id) const { auto it=chunks_.find(id); return it==chunks_.end()?0:it->second.version; }
}
