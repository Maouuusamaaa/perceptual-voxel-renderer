#include "mesh/greedy_mesher.hpp"
#include <algorithm>
#include <stdexcept>

namespace pvr {

namespace {
struct Cell { std::uint16_t material{}; int sign{}; };

std::size_t indexOf(const VoxelSection& v, std::uint32_t x, std::uint32_t y, std::uint32_t z) {
    return (std::size_t(y) * v.sz() + z) * v.sx() + x;
}

bool inBounds(const VoxelSection& v, int x, int y, int z) {
    return x >= 0 && y >= 0 && z >= 0 &&
           x < static_cast<int>(v.sx()) && y < static_cast<int>(v.sy()) && z < static_cast<int>(v.sz());
}

std::uint16_t sample(const VoxelSection& v, int x, int y, int z) {
    return inBounds(v, x, y, z) ? v.get(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y), static_cast<std::uint32_t>(z)) : 0;
}

void addMaterial(Mesh& mesh, std::uint16_t material) {
    if (std::find(mesh.materials.begin(), mesh.materials.end(), material) == mesh.materials.end())
        mesh.materials.push_back(material);
}

void emitQuad(Mesh& mesh, int axis, int plane, int u, int v, int w, int h, int sign, std::uint16_t material) {
    Vertex p[4]{};
    if (axis == 0) {
        p[0] = {float(plane), float(u),     float(v),     std::int8_t(sign), 0, 0, material};
        p[1] = {float(plane), float(u+w),   float(v),     std::int8_t(sign), 0, 0, material};
        p[2] = {float(plane), float(u+w),   float(v+h),   std::int8_t(sign), 0, 0, material};
        p[3] = {float(plane), float(u),     float(v+h),   std::int8_t(sign), 0, 0, material};
    } else if (axis == 1) {
        p[0] = {float(u),     float(plane), float(v),     0, std::int8_t(sign), 0, material};
        p[1] = {float(u+w),   float(plane), float(v),     0, std::int8_t(sign), 0, material};
        p[2] = {float(u+w),   float(plane), float(v+h),   0, std::int8_t(sign), 0, material};
        p[3] = {float(u),     float(plane), float(v+h),   0, std::int8_t(sign), 0, material};
    } else {
        p[0] = {float(u),     float(v),     float(plane), 0, 0, std::int8_t(sign), material};
        p[1] = {float(u+w),   float(v),     float(plane), 0, 0, std::int8_t(sign), material};
        p[2] = {float(u+w),   float(v+h),   float(plane), 0, 0, std::int8_t(sign), material};
        p[3] = {float(u),     float(v+h),   float(plane), 0, 0, std::int8_t(sign), material};
    }
    const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
    if (sign > 0) {
        mesh.vertices.insert(mesh.vertices.end(), std::begin(p), std::end(p));
        mesh.indices.insert(mesh.indices.end(), {base, base+1, base+2, base, base+2, base+3});
    } else {
        mesh.vertices.insert(mesh.vertices.end(), {p[0], p[3], p[2], p[1]});
        mesh.indices.insert(mesh.indices.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    ++mesh.quadCount;
    addMaterial(mesh, material);
}
}

VoxelSection::VoxelSection(std::uint32_t sx, std::uint32_t sy, std::uint32_t sz)
    : x_(sx), y_(sy), z_(sz), data_(std::size_t(sx) * sy * sz) {
    if (sx == 0 || sy == 0 || sz == 0) throw std::invalid_argument("voxel section dimensions must be non-zero");
}

void VoxelSection::set(std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint16_t v) {
    if (x >= x_ || y >= y_ || z >= z_) throw std::out_of_range("voxel coordinate");
    data_[indexOf(*this, x, y, z)] = v;
}

std::uint16_t VoxelSection::get(std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
    if (x >= x_ || y >= y_ || z >= z_) throw std::out_of_range("voxel coordinate");
    return data_[indexOf(*this, x, y, z)];
}

std::uint32_t VoxelSection::sx() const noexcept { return x_; }
std::uint32_t VoxelSection::sy() const noexcept { return y_; }
std::uint32_t VoxelSection::sz() const noexcept { return z_; }

Mesh GreedyMesher::build(const VoxelSection& vox) {
    Mesh mesh;
    const int dims[3] = {static_cast<int>(vox.sx()), static_cast<int>(vox.sy()), static_cast<int>(vox.sz())};

    for (int axis = 0; axis < 3; ++axis) {
        const int uAxis = (axis + 1) % 3;
        const int vAxis = (axis + 2) % 3;
        const int du = dims[uAxis];
        const int dv = dims[vAxis];
        std::vector<Cell> mask(static_cast<std::size_t>(du) * dv);

        for (int slice = -1; slice < dims[axis]; ++slice) {
            std::fill(mask.begin(), mask.end(), Cell{});
            for (int u = 0; u < du; ++u) {
                for (int v = 0; v < dv; ++v) {
                    int a[3]{}, b[3]{};
                    a[axis] = slice; b[axis] = slice + 1;
                    a[uAxis] = b[uAxis] = u;
                    a[vAxis] = b[vAxis] = v;
                    const auto va = sample(vox, a[0], a[1], a[2]);
                    const auto vb = sample(vox, b[0], b[1], b[2]);
                    auto& c = mask[std::size_t(u) * dv + v];
                    if ((va != 0) == (vb != 0)) continue;
                    c.material = va != 0 ? va : vb;
                    c.sign = va != 0 ? 1 : -1;
                }
            }

            for (int u = 0; u < du; ++u) {
                for (int v = 0; v < dv; ) {
                    const auto c = mask[std::size_t(u) * dv + v];
                    if (c.sign == 0) { ++v; continue; }
                    int w = 1;
                    while (v + w < dv && mask[std::size_t(u) * dv + v + w].material == c.material && mask[std::size_t(u) * dv + v + w].sign == c.sign) ++w;
                    int h = 1;
                    bool stop = false;
                    while (u + h < du && !stop) {
                        for (int x = 0; x < w; ++x) {
                            const auto next = mask[std::size_t(u+h) * dv + v+x];
                            if (next.material != c.material || next.sign != c.sign) { stop = true; break; }
                        }
                        if (!stop) ++h;
                    }
                    for (int x = 0; x < h; ++x)
                        for (int y = 0; y < w; ++y)
                            mask[std::size_t(u+x) * dv + v+y] = Cell{};
                    emitQuad(mesh, axis, slice + 1, u, v, h, w, c.sign, c.material);
                    v += w;
                }
            }
        }
    }
    return mesh;
}
}
