#include "gpu/gpu_scene.hpp"
#include <algorithm>
#include <stdexcept>

namespace pvr {

std::size_t GPUScene::upsert(const GPUInstance& instance) {
    const auto it = std::find_if(instances_.begin(), instances_.end(),
        [&](const GPUInstance& existing) { return existing.node_id == instance.node_id; });
    if (it != instances_.end()) {
        *it = instance;
        return static_cast<std::size_t>(std::distance(instances_.begin(), it));
    }
    instances_.push_back(instance);
    return instances_.size() - 1;
}

void GPUScene::clear() noexcept { instances_.clear(); }
std::size_t GPUScene::size() const noexcept { return instances_.size(); }
const GPUInstance& GPUScene::at(std::size_t index) const {
    if (index >= instances_.size()) throw std::out_of_range("GPUScene index");
    return instances_[index];
}

}
