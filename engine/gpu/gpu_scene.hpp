#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace pvr {

struct GPUInstance {
    std::uint64_t node_id{};
    std::uint32_t geometry_id{};
    std::uint32_t material_id{};
    std::uint32_t lod{};
    float importance{};
};

class GPUScene {
public:
    std::size_t upsert(const GPUInstance& instance);
    void clear() noexcept;
    std::size_t size() const noexcept;
    const GPUInstance& at(std::size_t index) const;
private:
    std::vector<GPUInstance> instances_;
};

}
