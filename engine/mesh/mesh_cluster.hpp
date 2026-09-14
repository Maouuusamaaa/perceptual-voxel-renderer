#pragma once

#include <cstdint>
#include <vector>

namespace pvr {

struct MeshCluster {
    std::uint32_t first_index{};
    std::uint32_t index_count{};
    std::uint32_t first_vertex{};
    std::uint32_t vertex_count{};
    std::uint16_t material{};

    float min_x{};
    float min_y{};
    float min_z{};
    float max_x{};
    float max_y{};
    float max_z{};

    // Bounding sphere derived from the cluster AABB.
    float center_x{};
    float center_y{};
    float center_z{};
    float radius{};
};

}

namespace pvr {

struct Frustum {
    float min_x{};
    float max_x{};
    float min_y{};
    float max_y{};
    float min_z{};
    float max_z{};
};

struct VisibleMeshCluster {
    std::uint32_t cluster_id{};
    std::uint32_t first_index{};
    std::uint32_t index_count{};
    std::uint32_t first_vertex{};
    std::uint32_t vertex_count{};
    std::uint16_t material{};
};

struct PreparedDrawCommand {
    std::uint32_t index_count{};
    std::uint32_t instance_count{};
    std::uint32_t first_index{};
    std::int32_t vertex_offset{};
    std::uint32_t first_instance{};
};

struct IndirectDrawCommand {
    std::uint32_t index_count{};
    std::uint32_t instance_count{};
    std::uint32_t first_index{};
    std::int32_t vertex_offset{};
    std::uint32_t first_instance{};
};

struct GpuUploadSnapshot {
    std::uint64_t generation{};
    std::size_t size{};
    std::uint64_t checksum{};
    std::vector<std::uint8_t> payload;

    bool validate() const {
        /*
         * An empty snapshot represents a valid inactive-upload state.
         * Generation may be zero after reset or non-zero after a
         * failed upload that preserved the generation counter.
         */
        if (size == 0 &&
            checksum == 0 &&
            payload.empty()) {
            return true;
        }

        /*
         * A non-empty snapshot requires an active generation.
         */
        if (generation == 0) {
            return false;
        }

        /*
         * Size must describe the exact payload.
         */
        if (size != payload.size()) {
            return false;
        }

        /*
         * Recompute the same deterministic FNV-1a checksum
         * used by GpuUploadBackend::uploaded_checksum().
         */
        std::uint64_t hash = 14695981039346656037ull;

        for (std::uint8_t byte : payload) {
            hash ^= static_cast<std::uint64_t>(byte);
            hash *= 1099511628211ull;
        }

        return checksum == hash;
    }
};

class IndirectBuffer;

class GpuUploadBackend {
public:

    bool upload(
        const IndirectBuffer& buffer,
        bool success = true
    );

    std::uint64_t upload_generation() const {
        return upload_generation_;
    }

    bool has_generation(std::uint64_t generation) const {
        return generation != 0 &&
               generation == upload_generation_ &&
               has_upload_;
    }
    bool upload_validated(
        const IndirectBuffer& buffer,
        std::uint32_t mesh_index_count
    );

    bool upload(
        const std::vector<std::uint8_t>& payload,
        bool success = true
    ) {
        if (!success) {
            uploaded_payload_.clear();
            has_upload_ = false;
            return false;
        }

        uploaded_payload_ = payload;
        has_upload_ = true;
        ++upload_generation_;
        return true;
    }

    bool has_upload() const {
        return has_upload_;
    }

    const std::vector<std::uint8_t>& uploaded_payload() const {
        return uploaded_payload_;
    }

    std::size_t uploaded_size() const {
        return uploaded_payload_.size();
    }

    std::uint64_t uploaded_checksum() const {
        if (!has_upload_) {
            return 0;
        }

        std::uint64_t hash = 14695981039346656037ull;

        for (std::uint8_t byte : uploaded_payload_) {
            hash ^= static_cast<std::uint64_t>(byte);
            hash *= 1099511628211ull;
        }

        return hash;
    }

    bool restore_snapshot(const GpuUploadSnapshot& snapshot) {
        if (!snapshot.validate()) {
            return false;
        }

        if (snapshot.size == 0 &&
            snapshot.checksum == 0 &&
            snapshot.payload.empty()) {
            reset();
            upload_generation_ = snapshot.generation;
            return true;
        }

        uploaded_payload_ = snapshot.payload;
        has_upload_ = true;
        upload_generation_ = snapshot.generation;
        return true;
    }

    GpuUploadSnapshot snapshot() const {
        GpuUploadSnapshot result;
        result.generation = upload_generation_;

        if (!has_upload_) {
            return result;
        }

        result.size = uploaded_payload_.size();
        result.checksum = uploaded_checksum();
        result.payload = uploaded_payload_;
        return result;
    }

    void reset() {
        uploaded_payload_.clear();
        has_upload_ = false;
        upload_generation_ = 0;
    }

private:
    std::vector<std::uint8_t> uploaded_payload_;
    bool has_upload_{false};
    std::uint64_t upload_generation_{0};
};

class IndirectBuffer {
public:
    explicit IndirectBuffer(
        const std::vector<IndirectDrawCommand>& commands
    )
        : commands_(commands) {}

    std::size_t size() const {
        return commands_.size();
    }

    bool empty() const {
        return commands_.empty();
    }

    const IndirectDrawCommand& operator[](
        std::size_t index
    ) const {
        return commands_[index];
    }

    const std::vector<IndirectDrawCommand>& commands() const {
        return commands_;
    }

    bool validate(std::uint32_t mesh_index_count) const {
        for (const IndirectDrawCommand& command : commands_) {
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

    std::vector<std::uint8_t> upload_payload() const {
        return to_bytes();
    }

    std::vector<std::uint8_t> to_bytes() const {
        std::vector<std::uint8_t> bytes;
        bytes.reserve(commands_.size() * sizeof(std::uint32_t) * 5);

        auto append_u32 = [&bytes](std::uint32_t value) {
            bytes.push_back(
                static_cast<std::uint8_t>(value & 0xffu));
            bytes.push_back(
                static_cast<std::uint8_t>((value >> 8u) & 0xffu));
            bytes.push_back(
                static_cast<std::uint8_t>((value >> 16u) & 0xffu));
            bytes.push_back(
                static_cast<std::uint8_t>((value >> 24u) & 0xffu));
        };

        for (const IndirectDrawCommand& command : commands_) {
            append_u32(command.index_count);
            append_u32(command.instance_count);
            append_u32(command.first_index);
            append_u32(
                static_cast<std::uint32_t>(command.vertex_offset));
            append_u32(command.first_instance);
        }

        return bytes;
    }

private:
    std::vector<IndirectDrawCommand> commands_;
};

inline bool GpuUploadBackend::upload_validated(
    const IndirectBuffer& buffer,
    std::uint32_t mesh_index_count
) {
    if (!buffer.validate(mesh_index_count)) {
        return false;
    }

    return upload(buffer, true);
}


inline bool GpuUploadBackend::upload(
    const IndirectBuffer& buffer,
    bool success
) {
    return upload(buffer.upload_payload(), success);
}

struct DrawCommandBatch {
    std::uint16_t material{};
    std::uint32_t first_command{};
    std::uint32_t command_count{};
};

class MeshClusterCuller {
public:
    static bool is_visible(
        const MeshCluster& cluster,
        const Frustum& frustum
    );

    static std::vector<std::uint32_t> cull_visible(
        const std::vector<MeshCluster>& clusters,
        const Frustum& frustum
    );

    static std::vector<VisibleMeshCluster> compact_visible(
        const std::vector<MeshCluster>& clusters,
        const std::vector<std::uint32_t>& visible_indices
    );

    static std::vector<PreparedDrawCommand> prepare_draw_commands(
        const std::vector<VisibleMeshCluster>& visible
    );

    static std::vector<DrawCommandBatch> prepare_draw_batches(
        const std::vector<VisibleMeshCluster>& visible,
        const std::vector<PreparedDrawCommand>& commands
    );

    static std::vector<IndirectDrawCommand>
    prepare_indirect_draw_buffer(
        const std::vector<PreparedDrawCommand>& commands
    );

    static bool validate_indirect_draw_buffer(
        const std::vector<IndirectDrawCommand>& commands,
        std::uint32_t mesh_index_count
    );

    static bool validate_draw_commands(
        const std::vector<PreparedDrawCommand>& commands,
        std::uint32_t mesh_index_count
    );
};

}
