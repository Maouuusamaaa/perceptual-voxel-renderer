#include "renderer/gpu_pipeline.hpp"
namespace pvr {
void GPUCommandBuilder::add(const GPUInstance& i, bool visible) {
    if (!visible) return;
    commands_.push_back({i.node_id, i.geometry_id, i.material_id, i.lod, 1});
}
std::vector<GPUIndirectCommand> GPUCommandBuilder::build() const {
    return commands_;
}

GPUIndirectBatch GPUCommandBuilder::build_batch() const {
    GPUIndirectBatch batch;
    batch.commands.reserve(commands_.size());
    batch.metadata.reserve(commands_.size());

    for (std::size_t i = 0; i < commands_.size(); ++i) {
        const auto& source = commands_[i];

        GPUIndexedIndirectCommand command{
            36u,
            source.instance_count,
            0u,
            0,
            static_cast<std::uint32_t>(i)
        };

        batch.commands.push_back(command);
        batch.metadata.push_back({
            command,
            source.geometry_id,
            source.material_id
        });
    }

    batch.draw_count =
        static_cast<std::uint32_t>(batch.commands.size());

    return batch;
}

GPUIndirectBatch GPUCommandBuilder::build_batch(
    const std::vector<MeshCluster>& clusters
) const {
    GPUIndirectBatch batch;
    batch.commands.reserve(commands_.size());
    batch.metadata.reserve(commands_.size());

    for (std::size_t i = 0; i < commands_.size(); ++i) {
        const auto& source = commands_[i];

        if (source.geometry_id >= clusters.size()) {
            batch.rejected = true;
            return batch;
        }

        const auto& cluster = clusters[source.geometry_id];

        GPUIndexedIndirectCommand command{
            cluster.index_count,
            source.instance_count,
            cluster.first_index,
            static_cast<std::int32_t>(cluster.first_vertex),
            static_cast<std::uint32_t>(i)
        };

        batch.commands.push_back(command);
        batch.metadata.push_back({
            command,
            source.geometry_id,
            source.material_id
        });
    }

    batch.draw_count =
        static_cast<std::uint32_t>(batch.commands.size());

    return batch;
}

std::optional<IndirectBuffer>
GPUCommandBuilder::build_indirect_buffer(
    const std::vector<MeshCluster>& clusters
) const {
    const GPUIndirectBatch batch = build_batch(clusters);

    if (batch.rejected || !batch.validate()) {
        return std::nullopt;
    }

    std::vector<IndirectDrawCommand> commands;
    commands.reserve(batch.commands.size());

    for (const auto& command : batch.commands) {
        IndirectDrawCommand indirect{
            command.index_count,
            command.instance_count,
            command.first_index,
            command.vertex_offset,
            command.first_instance
        };

        commands.push_back(indirect);
    }

    return IndirectBuffer(commands);
}

void GPUCommandBuilder::clear() {
    commands_.clear();
}
}
