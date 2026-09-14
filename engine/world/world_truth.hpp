#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace pvr {
struct ChunkId {
    std::int64_t x{}, y{}, z{};
    bool operator==(const ChunkId&) const = default;
    std::string toString() const;
};
struct ChunkIdHash { std::size_t operator()(const ChunkId& id) const noexcept; };

class WorldTruth {
public:
    explicit WorldTruth(std::uint64_t seed, std::uint32_t sectionSize = 16);
    std::uint64_t seed() const noexcept;
    void setBlock(ChunkId id, std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint16_t value);
    std::uint16_t getBlock(ChunkId id, std::uint32_t x, std::uint32_t y, std::uint32_t z) const;
    bool isDirty(ChunkId id) const;
    void clearDirty(ChunkId id);
    std::uint64_t chunkVersion(ChunkId id) const;
private:
    struct Chunk { std::unordered_map<std::uint64_t, std::uint16_t> blocks; bool dirty=false; std::uint64_t version=0; };
    std::uint64_t seed_;
    std::uint32_t sectionSize_;
    std::unordered_map<ChunkId, Chunk, ChunkIdHash> chunks_;
    static std::uint64_t key(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept;
};
}
