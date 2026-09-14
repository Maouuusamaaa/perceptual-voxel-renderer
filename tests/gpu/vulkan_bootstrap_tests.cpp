#include "renderer/vulkan_backend.hpp"
#include <iostream>

int main() {
    pvr::VulkanBackend backend;
    const auto result = backend.initialize();
    if (!result.ok) {
        std::cerr << result.message << '\n';
        return 1;
    }
    if (backend.physical_device_count() == 0) {
        std::cerr << "Vulkan initialized but no physical device was enumerated\n";
        return 2;
    }
    const auto devices = backend.physical_devices();
    if (devices.size() != backend.physical_device_count()) return 3;
    if (devices.front().name.empty()) return 4;
    std::cout << "Vulkan bootstrap: " << devices.size() << " physical device(s)\n";
    for (const auto& d : devices) std::cout << "- " << d.name << " Vulkan API " << d.api_version << '\n';
    return 0;
}
