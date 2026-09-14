#include "mesh/mesh_cluster.hpp"
#include "renderer/gpu_pipeline.hpp"
#include "renderer/gpu_draw.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>


static void cycle42_37_gpu_draw_metadata_contract() {
    pvr::GPUIndexedIndirectCommand command{
        36,
        1,
        0,
        0,
        0
    };

    /*
     * Vulkan's indexed indirect command remains a strict 20-byte ABI.
     * Renderer metadata must live beside it rather than inside it.
     */
    pvr::GPUDrawMetadata draw{
        command,
        321,
        17
    };

    assert(draw.command.index_count == 36);
    assert(draw.command.instance_count == 1);
    assert(draw.geometry_id == 321);
    assert(draw.material_id == 17);
}


static void cycle42_38_indirect_batch_metadata_contract() {
    pvr::GPUIndirectBatch batch;

    pvr::GPUIndexedIndirectCommand command{
        36,
        1,
        0,
        0,
        0
    };

    batch.commands.push_back(command);
    batch.draw_count = 1;

    /*
     * Every indirect draw must have matching renderer metadata.
     */
    batch.metadata.push_back({
        command,
        321,
        17
    });

    assert(batch.commands.size() == 1);
    assert(batch.metadata.size() == batch.commands.size());

    assert(batch.metadata[0].geometry_id == 321);
    assert(batch.metadata[0].material_id == 17);
    assert(batch.metadata[0].command.index_count == 36);
}


static void cycle42_39_indirect_batch_consistency_contract() {
    pvr::GPUIndirectBatch batch;

    pvr::GPUIndexedIndirectCommand command{
        36,
        1,
        0,
        0,
        0
    };

    batch.commands.push_back(command);
    batch.metadata.push_back({
        command,
        321,
        17
    });
    batch.draw_count = 1;

    assert(batch.validate());

    /*
     * A mismatch between commands and metadata must invalidate the batch.
     */
    batch.metadata.clear();
    assert(!batch.validate());

    /*
     * Restore metadata, then invalidate draw_count.
     */
    batch.metadata.push_back({
        command,
        321,
        17
    });
    batch.draw_count = 0;
    assert(!batch.validate());

    /*
     * Empty batch with zero draw count is valid.
     */
    batch.commands.clear();
    batch.metadata.clear();
    batch.draw_count = 0;
    assert(batch.validate());
}


static void cycle42_40_gpu_command_builder_batch_contract() {
    pvr::GPUCommandBuilder builder;

    pvr::GPUInstance instance{
        88,
        321,
        17,
        0,
        5.0f
    };

    builder.add(instance, true);

    auto batch = builder.build_batch();

    assert(batch.validate());
    assert(batch.commands.size() == 1);
    assert(batch.metadata.size() == 1);
    assert(batch.draw_count == 1);

    assert(batch.metadata[0].geometry_id == 321);
    assert(batch.metadata[0].material_id == 17);
    assert(batch.metadata[0].command.index_count > 0);
}


static void cycle42_41_indirect_batch_command_validation_contract() {
    pvr::GPUIndirectBatch batch;

    pvr::GPUIndexedIndirectCommand valid{
        36,
        1,
        0,
        0,
        0
    };

    batch.commands.push_back(valid);
    batch.metadata.push_back({
        valid,
        321,
        17
    });
    batch.draw_count = 1;

    assert(batch.validate());

    /*
     * Zero index_count is not a valid indirect draw command.
     */
    batch.commands[0].index_count = 0;
    batch.metadata[0].command.index_count = 0;
    assert(!batch.validate());

    /*
     * Restore index count, then reject zero instance_count.
     */
    batch.commands[0].index_count = 36;
    batch.metadata[0].command.index_count = 36;
    batch.commands[0].instance_count = 0;
    batch.metadata[0].command.instance_count = 0;
    assert(!batch.validate());

    /*
     * Restore instance count, then reject negative vertex offset.
     */
    batch.commands[0].instance_count = 1;
    batch.metadata[0].command.instance_count = 1;
    batch.commands[0].vertex_offset = -1;
    batch.metadata[0].command.vertex_offset = -1;
    assert(!batch.validate());
}


static void cycle42_42_multi_command_batch_integrity_contract() {
    pvr::GPUCommandBuilder builder;

    pvr::GPUInstance first{100, 501, 11, 0, 5.0f};
    pvr::GPUInstance second{200, 502, 22, 1, 4.0f};
    pvr::GPUInstance third{300, 503, 33, 2, 3.0f};

    builder.add(first, true);
    builder.add(second, true);
    builder.add(third, true);

    auto batch = builder.build_batch();

    assert(batch.validate());
    assert(batch.commands.size() == 3);
    assert(batch.metadata.size() == 3);
    assert(batch.draw_count == 3);

    assert(batch.metadata[0].geometry_id == 501);
    assert(batch.metadata[0].material_id == 11);
    assert(batch.metadata[0].command.first_instance == 0);

    assert(batch.metadata[1].geometry_id == 502);
    assert(batch.metadata[1].material_id == 22);
    assert(batch.metadata[1].command.first_instance == 1);

    assert(batch.metadata[2].geometry_id == 503);
    assert(batch.metadata[2].material_id == 33);
    assert(batch.metadata[2].command.first_instance == 2);
}


static void cycle42_43_geometry_id_draw_range_contract() {
    pvr::GPUCommandBuilder builder;

    /*
     * geometry_id follows the existing MeshCluster cluster_id/index
     * convention for this renderer boundary.
     *
     * Cluster 0: 12 indices, starts at index 10, vertex offset 20.
     * Cluster 1: 24 indices, starts at index 50, vertex offset 60.
     */
    std::vector<pvr::MeshCluster> clusters(2);

    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    clusters[1].first_index = 50;
    clusters[1].index_count = 24;
    clusters[1].first_vertex = 60;
    clusters[1].vertex_count = 16;
    clusters[1].material = 9;

    pvr::GPUInstance first{100, 0, 3, 0, 5.0f};
    pvr::GPUInstance second{200, 1, 9, 0, 4.0f};

    builder.add(first, true);
    builder.add(second, true);

    auto batch = builder.build_batch(clusters);

    assert(batch.validate());
    assert(batch.commands.size() == 2);
    assert(batch.metadata.size() == 2);
    assert(batch.draw_count == 2);

    /*
     * Geometry 0 must use its actual MeshCluster draw range.
     */
    assert(batch.commands[0].index_count == 12);
    assert(batch.commands[0].first_index == 10);
    assert(batch.commands[0].vertex_offset == 20);

    assert(batch.metadata[0].geometry_id == 0);
    assert(batch.metadata[0].material_id == 3);

    /*
     * Geometry 1 must use a different actual MeshCluster draw range.
     */
    assert(batch.commands[1].index_count == 24);
    assert(batch.commands[1].first_index == 50);
    assert(batch.commands[1].vertex_offset == 60);

    assert(batch.metadata[1].geometry_id == 1);
    assert(batch.metadata[1].material_id == 9);
}


static void cycle42_44_invalid_geometry_id_rejection_contract() {
    pvr::GPUCommandBuilder builder;

    /*
     * Only geometry 0 exists.
     */
    std::vector<pvr::MeshCluster> clusters(1);

    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    /*
     * geometry_id 7 does not exist.
     */
    pvr::GPUInstance invalid{
        100,
        7,
        3,
        0,
        5.0f
    };

    builder.add(invalid, true);

    auto batch = builder.build_batch(clusters);

    /*
     * An invalid geometry reference must not silently produce
     * a successful empty batch.
     */
    assert(!batch.validate());
}


static void cycle42_45_explicit_rejection_state_contract() {
    pvr::GPUIndirectBatch empty;
    assert(!empty.rejected);
    assert(empty.validate());

    pvr::GPUCommandBuilder builder;

    std::vector<pvr::MeshCluster> clusters(1);
    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    pvr::GPUInstance invalid{
        100,
        7,
        3,
        0,
        5.0f
    };

    builder.add(invalid, true);

    auto batch = builder.build_batch(clusters);

    assert(batch.rejected);
    assert(!batch.validate());
}


static void cycle42_46_gpu_batch_to_indirect_buffer_contract() {
    pvr::GPUCommandBuilder builder;

    std::vector<pvr::MeshCluster> clusters(1);
    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    pvr::GPUInstance valid{
        100,
        0,
        3,
        0,
        5.0f
    };

    builder.add(valid, true);

    auto buffer = builder.build_indirect_buffer(clusters);

    assert(buffer.has_value());
    assert(buffer->size() == 1);
    assert(buffer->operator[](0).index_count == 12);
    assert(buffer->operator[](0).instance_count == 1);
    assert(buffer->operator[](0).first_index == 10);
    assert(buffer->operator[](0).vertex_offset == 20);
    assert(buffer->validate(22));

    pvr::GPUCommandBuilder rejected_builder;

    pvr::GPUInstance invalid{
        200,
        7,
        3,
        0,
        5.0f
    };

    rejected_builder.add(invalid, true);

    auto rejected_buffer =
        rejected_builder.build_indirect_buffer(clusters);

    assert(!rejected_buffer.has_value());
}


static void cycle42_47_end_to_end_gpu_upload_contract() {
    pvr::GPUCommandBuilder builder;

    std::vector<pvr::MeshCluster> clusters(1);
    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    pvr::GPUInstance valid{
        100,
        0,
        3,
        0,
        5.0f
    };

    builder.add(valid, true);

    auto buffer = builder.build_indirect_buffer(clusters);

    assert(buffer.has_value());
    assert(buffer->validate(22));

    pvr::GpuUploadBackend backend;

    const auto expected_payload = buffer->upload_payload();

    assert(backend.upload(*buffer));
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == expected_payload);
    assert(backend.uploaded_size() == expected_payload.size());
    assert(backend.uploaded_checksum() != 0);
    assert(backend.upload_generation() == 1);
    assert(backend.has_generation(1));

    const auto first_checksum = backend.uploaded_checksum();

    assert(backend.upload(*buffer));
    assert(backend.upload_generation() == 2);
    assert(backend.has_generation(2));
    assert(backend.uploaded_checksum() == first_checksum);

    pvr::GPUCommandBuilder invalid_builder;

    pvr::GPUInstance invalid{
        200,
        7,
        3,
        0,
        5.0f
    };

    invalid_builder.add(invalid, true);

    auto invalid_buffer =
        invalid_builder.build_indirect_buffer(clusters);

    assert(!invalid_buffer.has_value());

    pvr::GpuUploadBackend rejected_backend;

    assert(!rejected_backend.has_upload());
    assert(rejected_backend.upload_generation() == 0);
}


static void cycle42_47_validated_gpu_upload_contract() {
    pvr::GPUCommandBuilder builder;

    std::vector<pvr::MeshCluster> clusters(1);
    clusters[0].first_index = 10;
    clusters[0].index_count = 12;
    clusters[0].first_vertex = 20;
    clusters[0].vertex_count = 8;
    clusters[0].material = 3;

    pvr::GPUInstance valid{
        100,
        0,
        3,
        0,
        5.0f
    };

    builder.add(valid, true);

    auto buffer = builder.build_indirect_buffer(clusters);

    assert(buffer.has_value());
    assert(buffer->validate(22));

    pvr::GpuUploadBackend backend;

    assert(backend.upload_validated(*buffer, 22));
    assert(backend.has_upload());
    assert(backend.upload_generation() == 1);

    const auto checksum = backend.uploaded_checksum();
    const auto size = backend.uploaded_size();

    assert(checksum != 0);
    assert(size > 0);

    assert(backend.upload_validated(*buffer, 22));
    assert(backend.upload_generation() == 2);
    assert(backend.uploaded_checksum() == checksum);
    assert(backend.uploaded_size() == size);

    pvr::IndirectDrawCommand invalid_command{
        12,
        1,
        20,
        0,
        0
    };

    pvr::IndirectBuffer invalid_buffer(
        std::vector<pvr::IndirectDrawCommand>{invalid_command}
    );

    assert(!invalid_buffer.validate(22));

    const auto generation_before_rejection =
        backend.upload_generation();

    assert(!backend.upload_validated(invalid_buffer, 22));
    assert(backend.upload_generation() ==
           generation_before_rejection);
    assert(backend.has_upload());
    assert(backend.uploaded_checksum() == checksum);
    assert(backend.uploaded_size() == size);
}

int main() {
    cycle42_47_validated_gpu_upload_contract();
    cycle42_47_end_to_end_gpu_upload_contract();
    cycle42_46_gpu_batch_to_indirect_buffer_contract();
    cycle42_45_explicit_rejection_state_contract();
    cycle42_44_invalid_geometry_id_rejection_contract();
    cycle42_43_geometry_id_draw_range_contract();
    cycle42_42_multi_command_batch_integrity_contract();
    cycle42_41_indirect_batch_command_validation_contract();
    cycle42_40_gpu_command_builder_batch_contract();
    cycle42_39_indirect_batch_consistency_contract();
    cycle42_38_indirect_batch_metadata_contract();
    cycle42_37_gpu_draw_metadata_contract();
    static_assert(std::is_standard_layout_v<pvr::GPUIndexedIndirectCommand>);
    static_assert(sizeof(pvr::GPUIndexedIndirectCommand) == 20);
    pvr::GPUIndirectBatch batch;
    batch.commands.push_back({36, 1, 0, 0, 0});
    batch.commands.push_back({24, 1, 36, 8, 1});
    batch.draw_count = static_cast<std::uint32_t>(batch.commands.size());
    assert(batch.draw_count == 2);
    assert(batch.commands[1].first_index == 36);
    std::cout << "gpu_draw_tests: PASS\n";
}
