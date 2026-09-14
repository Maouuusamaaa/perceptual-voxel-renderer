#include "mesh/mesh_cluster.hpp"

namespace pvr {

bool MeshClusterCuller::is_visible(
    const MeshCluster& cluster,
    const Frustum& frustum
) {
    /*
     * Bounding-sphere vs axis-aligned frustum test.
     *
     * The sphere is outside when its center is farther
     * than its radius beyond any frustum plane.
     */

    if (cluster.center_x + cluster.radius < frustum.min_x)
        return false;

    if (cluster.center_x - cluster.radius > frustum.max_x)
        return false;

    if (cluster.center_y + cluster.radius < frustum.min_y)
        return false;

    if (cluster.center_y - cluster.radius > frustum.max_y)
        return false;

    if (cluster.center_z + cluster.radius < frustum.min_z)
        return false;

    if (cluster.center_z - cluster.radius > frustum.max_z)
        return false;

    return true;
}

}

namespace pvr {

std::vector<std::uint32_t> MeshClusterCuller::cull_visible(
    const std::vector<MeshCluster>& clusters,
    const Frustum& frustum
) {
    std::vector<std::uint32_t> visible_indices;
    visible_indices.reserve(clusters.size());

    for (std::uint32_t i = 0;
         i < clusters.size();
         ++i) {
        if (is_visible(clusters[i], frustum)) {
            visible_indices.push_back(i);
        }
    }

    return visible_indices;
}

}

namespace pvr {

std::vector<VisibleMeshCluster> MeshClusterCuller::compact_visible(
    const std::vector<MeshCluster>& clusters,
    const std::vector<std::uint32_t>& visible_indices
) {
    std::vector<VisibleMeshCluster> compact;
    compact.reserve(visible_indices.size());

    for (std::uint32_t cluster_id : visible_indices) {
        if (cluster_id >= clusters.size()) {
            continue;
        }

        const MeshCluster& cluster = clusters[cluster_id];

        VisibleMeshCluster visible{};
        visible.cluster_id = cluster_id;
        visible.first_index = cluster.first_index;
        visible.index_count = cluster.index_count;
        visible.first_vertex = cluster.first_vertex;
        visible.vertex_count = cluster.vertex_count;
        visible.material = cluster.material;

        compact.push_back(visible);
    }

    return compact;
}

}

namespace pvr {

std::vector<PreparedDrawCommand> MeshClusterCuller::prepare_draw_commands(
    const std::vector<VisibleMeshCluster>& visible
) {
    std::vector<PreparedDrawCommand> commands;
    commands.reserve(visible.size());

    for (const VisibleMeshCluster& cluster : visible) {
        PreparedDrawCommand command{};

        command.index_count = cluster.index_count;
        command.instance_count = 1;
        command.first_index = cluster.first_index;
        command.vertex_offset =
            static_cast<std::int32_t>(cluster.first_vertex);
        command.first_instance = cluster.cluster_id;

        commands.push_back(command);
    }

    return commands;
}

}

namespace pvr {

bool MeshClusterCuller::validate_draw_commands(
    const std::vector<PreparedDrawCommand>& commands,
    std::uint32_t mesh_index_count
) {
    for (const PreparedDrawCommand& command : commands) {
        if (command.index_count == 0) {
            return false;
        }

        if (command.instance_count == 0) {
            return false;
        }

        if (command.vertex_offset < 0) {
            return false;
        }

        const std::uint64_t index_end =
            static_cast<std::uint64_t>(command.first_index) +
            static_cast<std::uint64_t>(command.index_count);

        if (index_end > mesh_index_count) {
            return false;
        }
    }

    return true;
}

}

namespace pvr {

std::vector<DrawCommandBatch> MeshClusterCuller::prepare_draw_batches(
    const std::vector<VisibleMeshCluster>& visible,
    const std::vector<PreparedDrawCommand>& commands
) {
    std::vector<DrawCommandBatch> batches;

    if (visible.size() != commands.size()) {
        return batches;
    }

    if (visible.empty()) {
        return batches;
    }

    batches.reserve(visible.size());

    std::uint16_t current_material = visible[0].material;
    std::uint32_t first_command = 0;
    std::uint32_t command_count = 1;

    for (std::size_t i = 1; i < visible.size(); ++i) {
        if (visible[i].material == current_material) {
            ++command_count;
            continue;
        }

        DrawCommandBatch batch{};
        batch.material = current_material;
        batch.first_command = first_command;
        batch.command_count = command_count;
        batches.push_back(batch);

        current_material = visible[i].material;
        first_command = static_cast<std::uint32_t>(i);
        command_count = 1;
    }

    DrawCommandBatch final_batch{};
    final_batch.material = current_material;
    final_batch.first_command = first_command;
    final_batch.command_count = command_count;
    batches.push_back(final_batch);

    return batches;
}

}

namespace pvr {

std::vector<IndirectDrawCommand>
MeshClusterCuller::prepare_indirect_draw_buffer(
    const std::vector<PreparedDrawCommand>& commands
) {
    std::vector<IndirectDrawCommand> buffer;
    buffer.reserve(commands.size());

    for (const PreparedDrawCommand& command : commands) {
        IndirectDrawCommand indirect{};
        indirect.index_count = command.index_count;
        indirect.instance_count = command.instance_count;
        indirect.first_index = command.first_index;
        indirect.vertex_offset = command.vertex_offset;
        indirect.first_instance = command.first_instance;

        buffer.push_back(indirect);
    }

    return buffer;
}

}

namespace pvr {

bool MeshClusterCuller::validate_indirect_draw_buffer(
    const std::vector<IndirectDrawCommand>& commands,
    std::uint32_t mesh_index_count
) {
    for (const IndirectDrawCommand& command : commands) {
        if (command.index_count == 0) {
            return false;
        }

        if (command.instance_count == 0) {
            return false;
        }

        if (command.vertex_offset < 0) {
            return false;
        }

        const std::uint64_t index_end =
            static_cast<std::uint64_t>(command.first_index) +
            static_cast<std::uint64_t>(command.index_count);

        if (index_end > mesh_index_count) {
            return false;
        }
    }

    return true;
}

}
