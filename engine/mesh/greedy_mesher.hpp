#pragma once
#include <cstdint>
#include <vector>

namespace pvr {

class VoxelSection {
public:
    VoxelSection(std::uint32_t sx, std::uint32_t sy, std::uint32_t sz);
    void set(std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint16_t value);
    std::uint16_t get(std::uint32_t x, std::uint32_t y, std::uint32_t z) const;
    std::uint32_t sx() const noexcept;
    std::uint32_t sy() const noexcept;
    std::uint32_t sz() const noexcept;
private:
    std::uint32_t x_, y_, z_;
    std::vector<std::uint16_t> data_;
};

struct Vertex {
    float x{}, y{}, z{};
    std::int8_t nx{}, ny{}, nz{};
    std::uint16_t material{};
};

struct Mesh {
    std::uint32_t quadCount{};
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<std::uint16_t> materials;
};

class GreedyMesher {
public:
    static Mesh build(const VoxelSection& vox);
};

}
