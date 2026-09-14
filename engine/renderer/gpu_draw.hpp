#pragma once
#include <cstdint>
#include <vector>

namespace pvr {
// ABI matching VkDrawIndexedIndirectCommand. Keep this POD so it can be
// copied directly into a Vulkan storage/indirect buffer once the real Vulkan
// backend is enabled.
struct GPUIndexedIndirectCommand {
    std::uint32_t index_count{};
    std::uint32_t instance_count{};
    std::uint32_t first_index{};
    std::int32_t vertex_offset{};
    std::uint32_t first_instance{};
};

struct GPUDrawMetadata {
    GPUIndexedIndirectCommand command{};
    std::uint32_t geometry_id{};
    std::uint32_t material_id{};
};

struct GPUIndirectBatch {
    std::vector<GPUIndexedIndirectCommand> commands;
    std::vector<GPUDrawMetadata> metadata;
    std::uint32_t draw_count{};
    bool rejected{false};

    bool validate() const {
        if (rejected) {
            return false;
        }
        if (commands.empty() && metadata.empty() && draw_count == 0) {
            return true;
        }

        if (commands.size() != metadata.size()) {
            return false;
        }

        if (draw_count != commands.size()) {
            return false;
        }

        for (std::size_t i = 0; i < commands.size(); ++i) {
            const auto& command = commands[i];
            const auto& metadata_command = metadata[i].command;

            if (command.index_count == 0) {
                return false;
            }

            if (command.instance_count == 0) {
                return false;
            }

            if (command.vertex_offset < 0) {
                return false;
            }

            if (metadata_command.index_count != command.index_count ||
                metadata_command.instance_count != command.instance_count ||
                metadata_command.first_index != command.first_index ||
                metadata_command.vertex_offset != command.vertex_offset ||
                metadata_command.first_instance != command.first_instance) {
                return false;
            }
        }

        return true;
    }
};
}
