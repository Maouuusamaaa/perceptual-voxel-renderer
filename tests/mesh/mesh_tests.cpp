#include "representation/representation.hpp"
#include "mesh/greedy_mesher.hpp"
#include "mesh/mesh_cluster.hpp"
#include "mesh/mesh_cluster_builder.hpp"
#include <cassert>
#include <iostream>

static void cycle42_mesh_cluster_contract() {
    using namespace pvr;

    MeshCluster cluster;
    cluster.first_index = 0;
    cluster.index_count = 6;
    cluster.first_vertex = 0;
    cluster.vertex_count = 4;
    cluster.material = 11;

    cluster.min_x = 0.0f;
    cluster.min_y = 0.0f;
    cluster.min_z = 0.0f;
    cluster.max_x = 2.0f;
    cluster.max_y = 1.0f;
    cluster.max_z = 2.0f;

    assert(cluster.first_index == 0);
    assert(cluster.index_count == 6);
    assert(cluster.first_vertex == 0);
    assert(cluster.vertex_count == 4);
    assert(cluster.material == 11);

    assert(cluster.min_x == 0.0f);
    assert(cluster.min_y == 0.0f);
    assert(cluster.min_z == 0.0f);
    assert(cluster.max_x == 2.0f);
    assert(cluster.max_y == 1.0f);
    assert(cluster.max_z == 2.0f);
}

static void cycle42_mesh_cluster_builder_contract() {
    using namespace pvr;

    VoxelSection section(2, 1, 2);
    section.set(0, 0, 0, 9);
    section.set(1, 0, 0, 9);
    section.set(0, 0, 1, 9);
    section.set(1, 0, 1, 9);

    const Mesh mesh = GreedyMesher::build(section);

    MeshClusterBuilder builder;
    const auto clusters = builder.build(mesh, 2);

    assert(clusters.size() == 3);

    assert(clusters[0].first_index == 0);
    assert(clusters[0].index_count == 12);

    assert(clusters[1].first_index == 12);
    assert(clusters[1].index_count == 12);

    assert(clusters[2].first_index == 24);
    assert(clusters[2].index_count == 12);

    assert(clusters[0].first_vertex == 0);
    assert(clusters[0].vertex_count == 8);

    assert(clusters[1].first_vertex == 8);
    assert(clusters[1].vertex_count == 8);

    assert(clusters[2].first_vertex == 16);
    assert(clusters[2].vertex_count == 8);

    assert(clusters[0].material == 9);
    assert(clusters[1].material == 9);
    assert(clusters[2].material == 9);
}

static void cycle42_mesh_cluster_bounding_sphere_contract() {
    using namespace pvr;

    VoxelSection section(2, 1, 2);

    section.set(0, 0, 0, 10);
    section.set(1, 0, 0, 10);
    section.set(0, 0, 1, 10);
    section.set(1, 0, 1, 10);

    const Mesh mesh = GreedyMesher::build(section);

    MeshClusterBuilder builder;
    const auto clusters = builder.build(mesh, 2);

    assert(!clusters.empty());

    for (const auto& cluster : clusters) {
        assert(cluster.radius >= 0.0f);

        const float center_x =
            cluster.min_x +
            (cluster.max_x - cluster.min_x) * 0.5f;

        const float center_y =
            cluster.min_y +
            (cluster.max_y - cluster.min_y) * 0.5f;

        const float center_z =
            cluster.min_z +
            (cluster.max_z - cluster.min_z) * 0.5f;

        assert(cluster.center_x == center_x);
        assert(cluster.center_y == center_y);
        assert(cluster.center_z == center_z);

        for (std::uint32_t i = 0;
             i < cluster.vertex_count;
             ++i) {

            const auto& vertex =
                mesh.vertices[
                    static_cast<std::size_t>(
                        cluster.first_vertex + i)];

            const float dx = vertex.x - cluster.center_x;
            const float dy = vertex.y - cluster.center_y;
            const float dz = vertex.z - cluster.center_z;

            const float distance_squared =
                dx * dx + dy * dy + dz * dz;

            assert(distance_squared <=
                   cluster.radius * cluster.radius + 0.0001f);
        }
    }
}

static void cycle42_mesh_cluster_determinism_contract() {
    using namespace pvr;

    VoxelSection section(3, 1, 3);

    for (std::uint32_t z = 0; z < 3; ++z) {
        for (std::uint32_t x = 0; x < 3; ++x) {
            section.set(x, 0, z, 12);
        }
    }

    const Mesh mesh = GreedyMesher::build(section);

    MeshClusterBuilder builder;

    const auto first = builder.build(mesh, 2);
    const auto second = builder.build(mesh, 2);

    assert(first.size() == second.size());

    for (std::size_t i = 0; i < first.size(); ++i) {
        const auto& a = first[i];
        const auto& b = second[i];

        assert(a.first_index == b.first_index);
        assert(a.index_count == b.index_count);
        assert(a.first_vertex == b.first_vertex);
        assert(a.vertex_count == b.vertex_count);
        assert(a.material == b.material);

        assert(a.min_x == b.min_x);
        assert(a.min_y == b.min_y);
        assert(a.min_z == b.min_z);
        assert(a.max_x == b.max_x);
        assert(a.max_y == b.max_y);
        assert(a.max_z == b.max_z);

        assert(a.center_x == b.center_x);
        assert(a.center_y == b.center_y);
        assert(a.center_z == b.center_z);
        assert(a.radius == b.radius);
    }

    // Every cluster must describe a valid contiguous vertex range.
    for (const auto& cluster : first) {
        assert(
            static_cast<std::size_t>(cluster.first_vertex) +
            static_cast<std::size_t>(cluster.vertex_count)
            <= mesh.vertices.size()
        );

        assert(
            static_cast<std::size_t>(cluster.first_index) +
            static_cast<std::size_t>(cluster.index_count)
            <= mesh.indices.size()
        );

        for (std::uint32_t i = 0;
             i < cluster.vertex_count;
             ++i) {

            const auto& vertex =
                mesh.vertices[
                    static_cast<std::size_t>(
                        cluster.first_vertex + i)];

            assert(vertex.x >= cluster.min_x);
            assert(vertex.x <= cluster.max_x);
            assert(vertex.y >= cluster.min_y);
            assert(vertex.y <= cluster.max_y);
            assert(vertex.z >= cluster.min_z);
            assert(vertex.z <= cluster.max_z);
        }
    }
}

static void cycle42_mesh_cluster_spatial_separation_contract() {
    using namespace pvr;

    Mesh mesh;

    /*
     * Four independent quads.
     *
     * Quad 0-1 are close together near X = 0.
     * Quad 2-3 are close together near X = 100.
     *
     * With two quads per cluster, each cluster should
     * describe only one spatial group.
     */

    auto add_quad = [&](float base_x) {
        const std::uint32_t first_vertex =
            static_cast<std::uint32_t>(mesh.vertices.size());

        const std::uint32_t first_index =
            static_cast<std::uint32_t>(mesh.indices.size());

        mesh.vertices.push_back(
            Vertex{base_x + 0.0f, 0.0f, 0.0f, 0, 1, 0, 7});
        mesh.vertices.push_back(
            Vertex{base_x + 1.0f, 0.0f, 0.0f, 0, 1, 0, 7});
        mesh.vertices.push_back(
            Vertex{base_x + 1.0f, 1.0f, 0.0f, 0, 1, 0, 7});
        mesh.vertices.push_back(
            Vertex{base_x + 0.0f, 1.0f, 0.0f, 0, 1, 0, 7});

        mesh.indices.push_back(first_vertex + 0);
        mesh.indices.push_back(first_vertex + 1);
        mesh.indices.push_back(first_vertex + 2);

        mesh.indices.push_back(first_vertex + 0);
        mesh.indices.push_back(first_vertex + 2);
        mesh.indices.push_back(first_vertex + 3);

        ++mesh.quadCount;
        mesh.materials.push_back(7);

        (void)first_index;
    };

    add_quad(0.0f);
    add_quad(2.0f);

    add_quad(100.0f);
    add_quad(102.0f);

    MeshClusterBuilder builder;

    const auto clusters = builder.build(mesh, 2);

    assert(clusters.size() == 2);

    const auto& first = clusters[0];
    const auto& second = clusters[1];

    /*
     * First cluster must cover only the first spatial group.
     */
    assert(first.min_x >= 0.0f);
    assert(first.max_x <= 3.0f);

    /*
     * Second cluster must cover only the distant spatial group.
     */
    assert(second.min_x >= 100.0f);
    assert(second.max_x <= 103.0f);

    /*
     * The clusters must not overlap spatially.
     */
    assert(first.max_x < second.min_x);

    /*
     * Bounding spheres must also remain separated.
     */
    const float center_distance =
        second.center_x - first.center_x;

    assert(center_distance >
           first.radius + second.radius);
}

static void cycle42_mesh_cluster_culling_contract() {
    using namespace pvr;

    MeshCluster visible{};
    visible.center_x = 0.0f;
    visible.center_y = 0.0f;
    visible.center_z = -5.0f;
    visible.radius = 1.0f;

    MeshCluster behind{};
    behind.center_x = 0.0f;
    behind.center_y = 0.0f;
    behind.center_z = 5.0f;
    behind.radius = 1.0f;

    MeshCluster intersecting{};
    intersecting.center_x = 0.0f;
    intersecting.center_y = 0.0f;
    intersecting.center_z = -0.5f;
    intersecting.radius = 1.0f;

    /*
     * Simple camera frustum:
     *
     * X: [-10, 10]
     * Y: [-10, 10]
     * Z: [-20, -1]
     *
     * Camera looks toward negative Z.
     */

    const Frustum frustum{
        -10.0f, 10.0f,
        -10.0f, 10.0f,
        -20.0f, -1.0f
    };

    assert(MeshClusterCuller::is_visible(visible, frustum));
    assert(!MeshClusterCuller::is_visible(behind, frustum));
    assert(MeshClusterCuller::is_visible(intersecting, frustum));
}

static void cycle42_mesh_cluster_culling_edge_cases_contract() {
    using namespace pvr;

    const Frustum frustum{
        -10.0f, 10.0f,
        -10.0f, 10.0f,
        -20.0f, -1.0f
    };

    /*
     * 1. Sphere exactly touching the near plane.
     * It must remain visible.
     */
    MeshCluster touching_near{};
    touching_near.center_x = 0.0f;
    touching_near.center_y = 0.0f;
    touching_near.center_z = -2.0f;
    touching_near.radius = 1.0f;

    assert(MeshClusterCuller::is_visible(
        touching_near,
        frustum
    ));

    /*
     * 2. Sphere exactly touching the far plane.
     * It must remain visible.
     */
    MeshCluster touching_far{};
    touching_far.center_x = 0.0f;
    touching_far.center_y = 0.0f;
    touching_far.center_z = -19.0f;
    touching_far.radius = 1.0f;

    assert(MeshClusterCuller::is_visible(
        touching_far,
        frustum
    ));

    /*
     * 3. Zero-radius point inside the frustum.
     */
    MeshCluster point_inside{};
    point_inside.center_x = 0.0f;
    point_inside.center_y = 0.0f;
    point_inside.center_z = -10.0f;
    point_inside.radius = 0.0f;

    assert(MeshClusterCuller::is_visible(
        point_inside,
        frustum
    ));

    /*
     * 4. Zero-radius point exactly on the boundary.
     */
    MeshCluster point_boundary{};
    point_boundary.center_x = 10.0f;
    point_boundary.center_y = 0.0f;
    point_boundary.center_z = -10.0f;
    point_boundary.radius = 0.0f;

    assert(MeshClusterCuller::is_visible(
        point_boundary,
        frustum
    ));

    /*
     * 5. Sphere completely outside +X.
     */
    MeshCluster outside_positive_x{};
    outside_positive_x.center_x = 12.0f;
    outside_positive_x.center_y = 0.0f;
    outside_positive_x.center_z = -10.0f;
    outside_positive_x.radius = 1.0f;

    assert(!MeshClusterCuller::is_visible(
        outside_positive_x,
        frustum
    ));

    /*
     * 6. Sphere completely outside -X.
     */
    MeshCluster outside_negative_x{};
    outside_negative_x.center_x = -12.0f;
    outside_negative_x.center_y = 0.0f;
    outside_negative_x.center_z = -10.0f;
    outside_negative_x.radius = 1.0f;

    assert(!MeshClusterCuller::is_visible(
        outside_negative_x,
        frustum
    ));

    /*
     * 7. Sphere completely outside +Y.
     */
    MeshCluster outside_positive_y{};
    outside_positive_y.center_x = 0.0f;
    outside_positive_y.center_y = 12.0f;
    outside_positive_y.center_z = -10.0f;
    outside_positive_y.radius = 1.0f;

    assert(!MeshClusterCuller::is_visible(
        outside_positive_y,
        frustum
    ));

    /*
     * 8. Sphere completely outside -Y.
     */
    MeshCluster outside_negative_y{};
    outside_negative_y.center_x = 0.0f;
    outside_negative_y.center_y = -12.0f;
    outside_negative_y.center_z = -10.0f;
    outside_negative_y.radius = 1.0f;

    assert(!MeshClusterCuller::is_visible(
        outside_negative_y,
        frustum
    ));

    /*
     * 9. Sphere completely in front of the near plane.
     */
    MeshCluster in_front{};
    in_front.center_x = 0.0f;
    in_front.center_y = 0.0f;
    in_front.center_z = 0.0f;
    in_front.radius = 0.5f;

    assert(!MeshClusterCuller::is_visible(
        in_front,
        frustum
    ));

    /*
     * 10. Sphere completely behind the far plane.
     */
    MeshCluster behind_far{};
    behind_far.center_x = 0.0f;
    behind_far.center_y = 0.0f;
    behind_far.center_z = -22.0f;
    behind_far.radius = 1.0f;

    assert(!MeshClusterCuller::is_visible(
        behind_far,
        frustum
    ));

    /*
     * 11. Large sphere crossing the entire frustum.
     * Even though its center is outside the near plane,
     * its radius intersects the frustum and therefore
     * it must remain visible.
     */
    MeshCluster large_intersection{};
    large_intersection.center_x = 0.0f;
    large_intersection.center_y = 0.0f;
    large_intersection.center_z = 0.0f;
    large_intersection.radius = 25.0f;

    assert(MeshClusterCuller::is_visible(
        large_intersection,
        frustum
    ));
}

static void cycle42_mesh_cluster_batch_culling_contract() {
    using namespace pvr;

    const Frustum frustum{
        -10.0f, 10.0f,
        -10.0f, 10.0f,
        -20.0f, -1.0f
    };

    std::vector<MeshCluster> clusters;

    /*
     * Cluster 0: visible.
     */
    MeshCluster visible_a{};
    visible_a.center_x = 0.0f;
    visible_a.center_y = 0.0f;
    visible_a.center_z = -5.0f;
    visible_a.radius = 1.0f;
    clusters.push_back(visible_a);

    /*
     * Cluster 1: outside +X.
     */
    MeshCluster outside_x{};
    outside_x.center_x = 20.0f;
    outside_x.center_y = 0.0f;
    outside_x.center_z = -5.0f;
    outside_x.radius = 1.0f;
    clusters.push_back(outside_x);

    /*
     * Cluster 2: visible.
     */
    MeshCluster visible_b{};
    visible_b.center_x = -5.0f;
    visible_b.center_y = 2.0f;
    visible_b.center_z = -10.0f;
    visible_b.radius = 2.0f;
    clusters.push_back(visible_b);

    /*
     * Cluster 3: behind camera.
     */
    MeshCluster behind{};
    behind.center_x = 0.0f;
    behind.center_y = 0.0f;
    behind.center_z = 5.0f;
    behind.radius = 1.0f;
    clusters.push_back(behind);

    /*
     * Cluster 4: visible at boundary.
     */
    MeshCluster boundary{};
    boundary.center_x = 10.0f;
    boundary.center_y = 0.0f;
    boundary.center_z = -10.0f;
    boundary.radius = 0.0f;
    clusters.push_back(boundary);

    const auto visible_indices =
        MeshClusterCuller::cull_visible(clusters, frustum);

    /*
     * Expected visible clusters:
     * 0, 2, 4
     */
    assert(visible_indices.size() == 3);

    assert(visible_indices[0] == 0);
    assert(visible_indices[1] == 2);
    assert(visible_indices[2] == 4);
}

static void cycle42_mesh_cluster_batch_culling_determinism_contract() {
    using namespace pvr;

    const Frustum frustum{
        -10.0f, 10.0f,
        -10.0f, 10.0f,
        -20.0f, -1.0f
    };

    /*
     * 1. Empty input must produce empty output.
     */
    const std::vector<MeshCluster> empty_clusters;

    const auto empty_result =
        MeshClusterCuller::cull_visible(
            empty_clusters,
            frustum
        );

    assert(empty_result.empty());

    /*
     * 2. All-visible input must preserve every index.
     */
    std::vector<MeshCluster> all_visible;

    for (int i = 0; i < 5; ++i) {
        MeshCluster cluster{};
        cluster.center_x = static_cast<float>(i - 2);
        cluster.center_y = 0.0f;
        cluster.center_z = -5.0f;
        cluster.radius = 0.5f;

        all_visible.push_back(cluster);
    }

    const auto all_visible_result =
        MeshClusterCuller::cull_visible(
            all_visible,
            frustum
        );

    assert(all_visible_result.size() == 5);

    for (std::uint32_t i = 0; i < 5; ++i) {
        assert(all_visible_result[i] == i);
    }

    /*
     * 3. All-culled input must produce empty output.
     */
    std::vector<MeshCluster> all_culled;

    for (int i = 0; i < 5; ++i) {
        MeshCluster cluster{};
        cluster.center_x = 50.0f + static_cast<float>(i);
        cluster.center_y = 0.0f;
        cluster.center_z = -5.0f;
        cluster.radius = 0.5f;

        all_culled.push_back(cluster);
    }

    const auto all_culled_result =
        MeshClusterCuller::cull_visible(
            all_culled,
            frustum
        );

    assert(all_culled_result.empty());

    /*
     * 4. Mixed input must preserve original ordering.
     *
     * Visible indices: 0, 2, 4
     */
    std::vector<MeshCluster> mixed;

    MeshCluster a{};
    a.center_x = 0.0f;
    a.center_y = 0.0f;
    a.center_z = -5.0f;
    a.radius = 1.0f;
    mixed.push_back(a);

    MeshCluster b{};
    b.center_x = 50.0f;
    b.center_y = 0.0f;
    b.center_z = -5.0f;
    b.radius = 1.0f;
    mixed.push_back(b);

    MeshCluster c{};
    c.center_x = -3.0f;
    c.center_y = 0.0f;
    c.center_z = -8.0f;
    c.radius = 1.0f;
    mixed.push_back(c);

    MeshCluster d{};
    d.center_x = -50.0f;
    d.center_y = 0.0f;
    d.center_z = -5.0f;
    d.radius = 1.0f;
    mixed.push_back(d);

    MeshCluster e{};
    e.center_x = 3.0f;
    e.center_y = 0.0f;
    e.center_z = -12.0f;
    e.radius = 1.0f;
    mixed.push_back(e);

    const auto mixed_result =
        MeshClusterCuller::cull_visible(
            mixed,
            frustum
        );

    assert(mixed_result.size() == 3);
    assert(mixed_result[0] == 0);
    assert(mixed_result[1] == 2);
    assert(mixed_result[2] == 4);

    /*
     * 5. Repeated calls must be deterministic.
     */
    const auto repeated_result =
        MeshClusterCuller::cull_visible(
            mixed,
            frustum
        );

    assert(repeated_result == mixed_result);
}

static void cycle42_visible_cluster_compaction_contract() {
    using namespace pvr;

    const Frustum frustum{
        -10.0f, 10.0f,
        -10.0f, 10.0f,
        -20.0f, -1.0f
    };

    std::vector<MeshCluster> clusters;

    MeshCluster a{};
    a.first_index = 10;
    a.index_count = 12;
    a.first_vertex = 20;
    a.vertex_count = 8;
    a.material = 3;
    a.center_x = 0.0f;
    a.center_y = 0.0f;
    a.center_z = -5.0f;
    a.radius = 1.0f;
    clusters.push_back(a);

    MeshCluster b{};
    b.first_index = 30;
    b.index_count = 18;
    b.first_vertex = 40;
    b.vertex_count = 12;
    b.material = 7;
    b.center_x = 50.0f;
    b.center_y = 0.0f;
    b.center_z = -5.0f;
    b.radius = 1.0f;
    clusters.push_back(b);

    MeshCluster c{};
    c.first_index = 50;
    c.index_count = 24;
    c.first_vertex = 60;
    c.vertex_count = 16;
    c.material = 9;
    c.center_x = -4.0f;
    c.center_y = 1.0f;
    c.center_z = -8.0f;
    c.radius = 1.0f;
    clusters.push_back(c);

    /*
     * Visible clusters are 0 and 2.
     *
     * The compact representation must contain exactly
     * those clusters and preserve their original order.
     */
    const auto visible =
        MeshClusterCuller::cull_visible(
            clusters,
            frustum
        );

    const auto compact =
        MeshClusterCuller::compact_visible(
            clusters,
            visible
        );

    assert(compact.size() == 2);

    assert(compact[0].cluster_id == 0);
    assert(compact[0].first_index == 10);
    assert(compact[0].index_count == 12);
    assert(compact[0].first_vertex == 20);
    assert(compact[0].vertex_count == 8);
    assert(compact[0].material == 3);

    assert(compact[1].cluster_id == 2);
    assert(compact[1].first_index == 50);
    assert(compact[1].index_count == 24);
    assert(compact[1].first_vertex == 60);
    assert(compact[1].vertex_count == 16);
    assert(compact[1].material == 9);

    /*
     * Compaction must preserve one-to-one correspondence
     * with the visible cluster indices.
     */
    assert(compact[0].cluster_id == visible[0]);
    assert(compact[1].cluster_id == visible[1]);
}

static void cycle42_compacted_draw_command_contract() {
    using namespace pvr;

    std::vector<VisibleMeshCluster> visible;

    VisibleMeshCluster a{};
    a.cluster_id = 4;
    a.first_index = 12;
    a.index_count = 36;
    a.first_vertex = 8;
    a.vertex_count = 24;
    a.material = 3;
    visible.push_back(a);

    VisibleMeshCluster b{};
    b.cluster_id = 9;
    b.first_index = 48;
    b.index_count = 18;
    b.first_vertex = 32;
    b.vertex_count = 12;
    b.material = 7;
    visible.push_back(b);

    /*
     * Renderer-agnostic draw command:
     *
     * index_count
     * instance_count
     * first_index
     * vertex_offset
     * first_instance
     */
    const auto commands =
        MeshClusterCuller::prepare_draw_commands(visible);

    assert(commands.size() == 2);

    assert(commands[0].index_count == 36);
    assert(commands[0].instance_count == 1);
    assert(commands[0].first_index == 12);
    assert(commands[0].vertex_offset == 8);
    assert(commands[0].first_instance == 4);

    assert(commands[1].index_count == 18);
    assert(commands[1].instance_count == 1);
    assert(commands[1].first_index == 48);
    assert(commands[1].vertex_offset == 32);
    assert(commands[1].first_instance == 9);
}



/* CYCLE #42.12 — Draw Command Validation */




/* CYCLE #42.12 — Draw Command Validation */
static void cycle42_draw_command_validation_contract() {
    using namespace pvr;

    std::vector<PreparedDrawCommand> valid_commands;

    PreparedDrawCommand a{};
    a.index_count = 36;
    a.instance_count = 1;
    a.first_index = 0;
    a.vertex_offset = 0;
    a.first_instance = 4;
    valid_commands.push_back(a);

    PreparedDrawCommand b{};
    b.index_count = 18;
    b.instance_count = 1;
    b.first_index = 36;
    b.vertex_offset = 24;
    b.first_instance = 9;
    valid_commands.push_back(b);

    // 36 + 18 = 54 indices.
    assert(
        MeshClusterCuller::validate_draw_commands(
            valid_commands,
            54
        )
    );

    // Empty command list is valid.
    const std::vector<PreparedDrawCommand> empty_commands;

    assert(
        MeshClusterCuller::validate_draw_commands(
            empty_commands,
            54
        )
    );

    // Zero index count is invalid.
    auto zero_index = valid_commands;
    zero_index[0].index_count = 0;

    assert(
        !MeshClusterCuller::validate_draw_commands(
            zero_index,
            54
        )
    );

    // Zero instance count is invalid.
    auto zero_instance = valid_commands;
    zero_instance[0].instance_count = 0;

    assert(
        !MeshClusterCuller::validate_draw_commands(
            zero_instance,
            54
        )
    );

    // Index range exceeding mesh index count is invalid.
    auto bad_index_range = valid_commands;
    bad_index_range[1].first_index = 40;
    bad_index_range[1].index_count = 18;

    assert(
        !MeshClusterCuller::validate_draw_commands(
            bad_index_range,
            54
        )
    );

    // Negative vertex offset is invalid.
    auto bad_vertex_offset = valid_commands;
    bad_vertex_offset[1].vertex_offset = -1;

    assert(
        !MeshClusterCuller::validate_draw_commands(
            bad_vertex_offset,
            54
        )
    );
}


/* CYCLE #42.13 — Draw Command Batching */
static void cycle42_draw_command_batching_contract() {
    using namespace pvr;

    std::vector<VisibleMeshCluster> visible;

    VisibleMeshCluster a{};
    a.cluster_id = 10;
    a.first_index = 0;
    a.index_count = 36;
    a.first_vertex = 0;
    a.vertex_count = 24;
    a.material = 3;
    visible.push_back(a);

    VisibleMeshCluster b{};
    b.cluster_id = 11;
    b.first_index = 36;
    b.index_count = 36;
    b.first_vertex = 24;
    b.vertex_count = 24;
    b.material = 3;
    visible.push_back(b);

    VisibleMeshCluster c{};
    c.cluster_id = 12;
    c.first_index = 72;
    c.index_count = 36;
    c.first_vertex = 48;
    c.vertex_count = 24;
    c.material = 7;
    visible.push_back(c);

    VisibleMeshCluster d{};
    d.cluster_id = 13;
    d.first_index = 108;
    d.index_count = 18;
    d.first_vertex = 72;
    d.vertex_count = 12;
    d.material = 7;
    visible.push_back(d);

    auto commands =
        MeshClusterCuller::prepare_draw_commands(visible);

    auto batches =
        MeshClusterCuller::prepare_draw_batches(
            visible,
            commands
        );

    /*
     * Materials 3 and 7 produce two deterministic batches.
     */
    assert(batches.size() == 2);

    assert(batches[0].material == 3);
    assert(batches[0].first_command == 0);
    assert(batches[0].command_count == 2);

    assert(batches[1].material == 7);
    assert(batches[1].first_command == 2);
    assert(batches[1].command_count == 2);

    /*
     * Empty input produces no batches.
     */
    const std::vector<VisibleMeshCluster> empty_visible;
    const std::vector<PreparedDrawCommand> empty_commands;

    auto empty_batches =
        MeshClusterCuller::prepare_draw_batches(
            empty_visible,
            empty_commands
        );

    assert(empty_batches.empty());

    /*
     * Mismatched input sizes must not produce
     * invalid batches.
     */
    std::vector<PreparedDrawCommand> mismatched_commands{
        commands[0]
    };

    auto mismatched_batches =
        MeshClusterCuller::prepare_draw_batches(
            visible,
            mismatched_commands
        );

    assert(mismatched_batches.empty());
}



/* CYCLE #42.14 — Indirect Draw Buffer Preparation */
static void cycle42_indirect_draw_buffer_contract() {
    using namespace pvr;

    std::vector<PreparedDrawCommand> commands;

    PreparedDrawCommand a{};
    a.index_count = 36;
    a.instance_count = 1;
    a.first_index = 0;
    a.vertex_offset = 0;
    a.first_instance = 10;
    commands.push_back(a);

    PreparedDrawCommand b{};
    b.index_count = 18;
    b.instance_count = 1;
    b.first_index = 36;
    b.vertex_offset = 24;
    b.first_instance = 11;
    commands.push_back(b);

    auto buffer =
        MeshClusterCuller::prepare_indirect_draw_buffer(commands);

    /*
     * One indirect command is produced for each
     * prepared draw command.
     */
    assert(buffer.size() == 2);

    /*
     * Command 0 must preserve every draw parameter.
     */
    assert(buffer[0].index_count == 36);
    assert(buffer[0].instance_count == 1);
    assert(buffer[0].first_index == 0);
    assert(buffer[0].vertex_offset == 0);
    assert(buffer[0].first_instance == 10);

    /*
     * Command 1 must preserve every draw parameter.
     */
    assert(buffer[1].index_count == 18);
    assert(buffer[1].instance_count == 1);
    assert(buffer[1].first_index == 36);
    assert(buffer[1].vertex_offset == 24);
    assert(buffer[1].first_instance == 11);

    /*
     * Empty input produces an empty indirect buffer.
     */
    const std::vector<PreparedDrawCommand> empty_commands;

    auto empty_buffer =
        MeshClusterCuller::prepare_indirect_draw_buffer(
            empty_commands
        );

    assert(empty_buffer.empty());
}



/* CYCLE #42.15 — Indirect Buffer Validation */
static void cycle42_indirect_buffer_validation_contract() {
    using namespace pvr;

    /*
     * Empty indirect buffer is valid.
     */
    const std::vector<IndirectDrawCommand> empty_buffer;

    assert(
        MeshClusterCuller::validate_indirect_draw_buffer(
            empty_buffer,
            54
        )
    );

    /*
     * Valid commands.
     */
    std::vector<IndirectDrawCommand> valid_buffer;

    IndirectDrawCommand a{};
    a.index_count = 36;
    a.instance_count = 1;
    a.first_index = 0;
    a.vertex_offset = 0;
    a.first_instance = 10;
    valid_buffer.push_back(a);

    IndirectDrawCommand b{};
    b.index_count = 18;
    b.instance_count = 1;
    b.first_index = 36;
    b.vertex_offset = 24;
    b.first_instance = 11;
    valid_buffer.push_back(b);

    assert(
        MeshClusterCuller::validate_indirect_draw_buffer(
            valid_buffer,
            54
        )
    );

    /*
     * Zero index count is invalid.
     */
    auto bad_index_count = valid_buffer;
    bad_index_count[0].index_count = 0;

    assert(
        !MeshClusterCuller::validate_indirect_draw_buffer(
            bad_index_count,
            54
        )
    );

    /*
     * Zero instance count is invalid.
     */
    auto bad_instance_count = valid_buffer;
    bad_instance_count[0].instance_count = 0;

    assert(
        !MeshClusterCuller::validate_indirect_draw_buffer(
            bad_instance_count,
            54
        )
    );

    /*
     * Index range outside the mesh is invalid.
     */
    auto bad_index_range = valid_buffer;
    bad_index_range[1].first_index = 48;
    bad_index_range[1].index_count = 18;

    assert(
        !MeshClusterCuller::validate_indirect_draw_buffer(
            bad_index_range,
            54
        )
    );

    /*
     * Negative vertex offset is invalid.
     */
    auto bad_vertex_offset = valid_buffer;
    bad_vertex_offset[1].vertex_offset = -1;

    assert(
        !MeshClusterCuller::validate_indirect_draw_buffer(
            bad_vertex_offset,
            54
        )
    );

    /*
     * Validation must inspect every command.
     */
    auto bad_second_command = valid_buffer;
    bad_second_command[1].index_count = 0;

    assert(
        !MeshClusterCuller::validate_indirect_draw_buffer(
            bad_second_command,
            54
        )
    );
}



/* CYCLE #42.16 — Backend-Neutral Indirect Buffer Abstraction */
static void cycle42_indirect_buffer_abstraction_contract() {
    using namespace pvr;

    std::vector<IndirectDrawCommand> commands;

    IndirectDrawCommand a{};
    a.index_count = 36;
    a.instance_count = 1;
    a.first_index = 0;
    a.vertex_offset = 0;
    a.first_instance = 10;
    commands.push_back(a);

    IndirectDrawCommand b{};
    b.index_count = 18;
    b.instance_count = 1;
    b.first_index = 36;
    b.vertex_offset = 24;
    b.first_instance = 11;
    commands.push_back(b);

    /*
     * The abstraction owns a copy of the indirect commands.
     */
    IndirectBuffer buffer(commands);

    assert(buffer.size() == 2);
    assert(!buffer.empty());

    /*
     * Command order and contents must be preserved.
     */
    assert(buffer[0].index_count == 36);
    assert(buffer[0].instance_count == 1);
    assert(buffer[0].first_index == 0);
    assert(buffer[0].vertex_offset == 0);
    assert(buffer[0].first_instance == 10);

    assert(buffer[1].index_count == 18);
    assert(buffer[1].instance_count == 1);
    assert(buffer[1].first_index == 36);
    assert(buffer[1].vertex_offset == 24);
    assert(buffer[1].first_instance == 11);

    /*
     * The abstraction must expose its command storage
     * without allowing the test to depend on a backend.
     */
    const auto& stored = buffer.commands();

    assert(stored.size() == 2);
    assert(stored[0].first_instance == 10);
    assert(stored[1].first_instance == 11);

    /*
     * Empty construction must be valid.
     */
    const std::vector<IndirectDrawCommand> empty_commands;
    IndirectBuffer empty_buffer(empty_commands);

    assert(empty_buffer.size() == 0);
    assert(empty_buffer.empty());
    assert(empty_buffer.commands().empty());
}



/* CYCLE #42.17 — Indirect Buffer Payload */
static void cycle42_indirect_buffer_payload_contract() {
    using namespace pvr;

    std::vector<IndirectDrawCommand> commands;

    IndirectDrawCommand a{};
    a.index_count = 36;
    a.instance_count = 1;
    a.first_index = 0;
    a.vertex_offset = 0;
    a.first_instance = 10;
    commands.push_back(a);

    IndirectDrawCommand b{};
    b.index_count = 18;
    b.instance_count = 1;
    b.first_index = 36;
    b.vertex_offset = 24;
    b.first_instance = 11;
    commands.push_back(b);

    IndirectBuffer buffer(commands);

    auto payload = buffer.to_bytes();

    /*
     * Each indirect command contains five 32-bit fields.
     */
    assert(payload.size() ==
           commands.size() * sizeof(std::uint32_t) * 5);

    /*
     * Serialization must be deterministic.
     */
    auto payload_again = buffer.to_bytes();

    assert(payload == payload_again);

    /*
     * Empty buffer produces an empty payload.
     */
    const std::vector<IndirectDrawCommand> empty_commands;
    IndirectBuffer empty_buffer(empty_commands);

    auto empty_payload = empty_buffer.to_bytes();

    assert(empty_payload.empty());
}



/* CYCLE #42.18 — IndirectBuffer object validation */
static void cycle42_indirect_buffer_object_validation_contract() {
    using namespace pvr;

    IndirectDrawCommand valid{};
    valid.index_count = 36;
    valid.instance_count = 1;
    valid.first_index = 0;
    valid.vertex_offset = 0;
    valid.first_instance = 10;

    IndirectBuffer valid_buffer(
        std::vector<IndirectDrawCommand>{valid});

    assert(valid_buffer.validate(36));

    /*
     * Validation must not mutate serialized payload.
     */
    auto before = valid_buffer.to_bytes();

    assert(valid_buffer.validate(36));

    auto after = valid_buffer.to_bytes();

    assert(before == after);

    /*
     * Invalid index count.
     */
    IndirectDrawCommand zero_index = valid;
    zero_index.index_count = 0;

    IndirectBuffer zero_index_buffer(
        std::vector<IndirectDrawCommand>{zero_index});

    assert(!zero_index_buffer.validate(36));

    /*
     * Invalid instance count.
     */
    IndirectDrawCommand zero_instance = valid;
    zero_instance.instance_count = 0;

    IndirectBuffer zero_instance_buffer(
        std::vector<IndirectDrawCommand>{zero_instance});

    assert(!zero_instance_buffer.validate(36));

    /*
     * Invalid negative vertex offset.
     */
    IndirectDrawCommand negative_vertex = valid;
    negative_vertex.vertex_offset = -1;

    IndirectBuffer negative_vertex_buffer(
        std::vector<IndirectDrawCommand>{negative_vertex});

    assert(!negative_vertex_buffer.validate(36));

    /*
     * Index range exceeds mesh.
     */
    IndirectDrawCommand out_of_range = valid;
    out_of_range.first_index = 30;
    out_of_range.index_count = 12;

    IndirectBuffer out_of_range_buffer(
        std::vector<IndirectDrawCommand>{out_of_range});

    assert(!out_of_range_buffer.validate(36));

    /*
     * Empty buffer is valid.
     */
    IndirectBuffer empty_buffer(
        std::vector<IndirectDrawCommand>{});

    assert(empty_buffer.validate(0));
}



/* CYCLE #42.19 — IndirectBuffer upload contract */
static void cycle42_indirect_buffer_upload_contract() {
    using namespace pvr;

    IndirectDrawCommand command{};
    command.index_count = 36;
    command.instance_count = 1;
    command.first_index = 0;
    command.vertex_offset = 0;
    command.first_instance = 42;

    IndirectBuffer buffer(
        std::vector<IndirectDrawCommand>{command});

    auto payload = buffer.upload_payload();

    auto serialized = buffer.to_bytes();

    /*
     * Upload payload must exactly match serialized buffer data.
     */
    assert(payload == serialized);

    /*
     * Payload size must match the serialized command count.
     */
    assert(payload.size() ==
           buffer.size() * sizeof(std::uint32_t) * 5);

    /*
     * Upload payload must be deterministic.
     */
    auto payload_again = buffer.upload_payload();

    assert(payload == payload_again);

    /*
     * Empty buffer produces an empty upload payload.
     */
    IndirectBuffer empty_buffer(
        std::vector<IndirectDrawCommand>{});

    auto empty_payload = empty_buffer.upload_payload();

    assert(empty_payload.empty());
}



/* CYCLE #42.20 — GPU upload backend interface */
static void cycle42_gpu_upload_backend_contract() {
    using namespace pvr;

    std::vector<std::uint8_t> payload{
        0x01, 0x02, 0x03, 0x04,
        0x10, 0x20, 0x30, 0x40
    };

    GpuUploadBackend backend;

    assert(!backend.has_upload());

    assert(backend.upload(payload));

    assert(backend.has_upload());

    const auto& uploaded = backend.uploaded_payload();

    assert(uploaded == payload);

    /*
     * Empty payload is still a valid upload operation.
     */
    std::vector<std::uint8_t> empty_payload;

    assert(backend.upload(empty_payload));

    assert(backend.has_upload());

    assert(backend.uploaded_payload().empty());
}



/* CYCLE #42.21 — GPU upload failure contract */
static void cycle42_gpu_upload_failure_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    std::vector<std::uint8_t> payload{
        0xAA, 0xBB, 0xCC, 0xDD
    };

    /*
     * The backend must be able to represent a failed upload.
     */
    assert(!backend.upload(payload, false));

    /*
     * A failed upload must not be reported as a successful upload.
     */
    assert(!backend.has_upload());

    /*
     * A successful upload must still work afterward.
     */
    assert(backend.upload(payload, true));
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == payload);
}



/* CYCLE #42.22 — GPU upload backend state reset */
static void cycle42_gpu_upload_backend_reset_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    std::vector<std::uint8_t> payload{
        0x11, 0x22, 0x33, 0x44
    };

    assert(backend.upload(payload));
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == payload);

    /*
     * Reset must return the backend to its initial state.
     */
    backend.reset();

    assert(!backend.has_upload());
    assert(backend.uploaded_payload().empty());

    /*
     * Backend must remain reusable after reset.
     */
    std::vector<std::uint8_t> second_payload{
        0xAA, 0xBB
    };

    assert(backend.upload(second_payload));
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == second_payload);
}



/* CYCLE #42.23 — GPU upload payload size contract */
static void cycle42_gpu_upload_payload_size_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    assert(backend.uploaded_size() == 0);

    std::vector<std::uint8_t> payload{
        0x01, 0x02, 0x03, 0x04, 0x05
    };

    assert(backend.upload(payload));
    assert(backend.has_upload());
    assert(backend.uploaded_size() == payload.size());

    std::vector<std::uint8_t> second_payload{
        0xAA, 0xBB
    };

    assert(backend.upload(second_payload));
    assert(backend.uploaded_size() == second_payload.size());

    backend.reset();

    assert(!backend.has_upload());
    assert(backend.uploaded_size() == 0);
}



/* CYCLE #42.24 — GPU upload payload immutability */
static void cycle42_gpu_upload_payload_immutability_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    std::vector<std::uint8_t> source{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(source));
    assert(backend.uploaded_payload() == source);

    /*
     * Mutating the source buffer after upload must not mutate
     * the backend's stored upload payload.
     */
    source[0] = 0xFF;
    source[1] = 0xEE;
    source.clear();

    std::vector<std::uint8_t> expected{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.uploaded_payload() == expected);
    assert(backend.uploaded_size() == expected.size());
}



/* CYCLE #42.25 — GPU upload payload integrity */
static void cycle42_gpu_upload_payload_integrity_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    std::vector<std::uint8_t> payload{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(payload));

    const std::uint64_t first_checksum =
        backend.uploaded_checksum();

    assert(first_checksum != 0);

    /*
     * The same payload must produce the same checksum.
     */
    GpuUploadBackend second_backend;

    assert(second_backend.upload(payload));

    assert(second_backend.uploaded_checksum() == first_checksum);

    /*
     * A different payload must produce a different checksum.
     */
    std::vector<std::uint8_t> different_payload{
        0x10, 0x20, 0x30, 0x41
    };

    assert(second_backend.upload(different_payload));

    assert(second_backend.uploaded_checksum() != first_checksum);

    /*
     * Reset must clear the integrity state.
     */
    second_backend.reset();

    assert(second_backend.uploaded_checksum() == 0);
}



/* CYCLE #42.26 — GPU upload generation contract */
static void cycle42_gpu_upload_generation_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    assert(backend.upload_generation() == 0);

    std::vector<std::uint8_t> first_payload{
        0x10, 0x20
    };

    assert(backend.upload(first_payload));
    assert(backend.upload_generation() == 1);

    std::vector<std::uint8_t> second_payload{
        0x30, 0x40, 0x50
    };

    assert(backend.upload(second_payload));
    assert(backend.upload_generation() == 2);

    /*
     * Failed uploads must not advance the generation.
     */
    assert(!backend.upload(second_payload, false));
    assert(backend.upload_generation() == 2);

    /*
     * Reset returns generation to the initial state.
     */
    backend.reset();

    assert(backend.upload_generation() == 0);
}



/* CYCLE #42.27 — generation-aware payload state contract */
static void cycle42_gpu_upload_generation_aware_state_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    assert(backend.upload_generation() == 0);
    assert(!backend.has_generation(0));
    assert(!backend.has_generation(1));

    std::vector<std::uint8_t> first_payload{
        0x10, 0x20, 0x30
    };

    assert(backend.upload(first_payload));

    const std::uint64_t first_generation =
        backend.upload_generation();

    assert(first_generation == 1);
    assert(backend.has_generation(first_generation));
    assert(!backend.has_generation(0));
    assert(!backend.has_generation(first_generation + 1));

    std::vector<std::uint8_t> second_payload{
        0x40, 0x50, 0x60
    };

    assert(backend.upload(second_payload));

    const std::uint64_t second_generation =
        backend.upload_generation();

    assert(second_generation == 2);
    assert(backend.has_generation(second_generation));
    assert(!backend.has_generation(first_generation));

    /*
     * Failed upload must preserve the currently committed generation.
     */
    assert(!backend.upload(second_payload, false));
    assert(backend.upload_generation() == second_generation);
    assert(!backend.has_generation(second_generation));

    backend.reset();

    assert(backend.upload_generation() == 0);
    assert(!backend.has_generation(0));
    assert(!backend.has_generation(second_generation));
}



/* CYCLE #42.28 — GPU upload snapshot contract */
static void cycle42_gpu_upload_snapshot_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    /*
     * Initial backend has no valid snapshot.
     */
    auto initial_snapshot = backend.snapshot();

    assert(initial_snapshot.generation == 0);
    assert(initial_snapshot.size == 0);
    assert(initial_snapshot.checksum == 0);
    assert(initial_snapshot.payload.empty());

    std::vector<std::uint8_t> payload{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(payload));

    auto snapshot = backend.snapshot();

    assert(snapshot.generation == backend.upload_generation());
    assert(snapshot.generation == 1);
    assert(snapshot.size == payload.size());
    assert(snapshot.checksum == backend.uploaded_checksum());
    assert(snapshot.checksum != 0);
    assert(snapshot.payload == payload);

    /*
     * Snapshot must remain a value copy of the uploaded state.
     */
    payload[0] = 0xFF;

    assert(snapshot.payload[0] == 0x10);

    /*
     * A new upload produces a new snapshot generation.
     */
    std::vector<std::uint8_t> second_payload{
        0x50, 0x60
    };

    assert(backend.upload(second_payload));

    auto second_snapshot = backend.snapshot();

    assert(second_snapshot.generation == 2);
    assert(second_snapshot.generation != snapshot.generation);
    assert(second_snapshot.size == second_payload.size());
    assert(second_snapshot.checksum == backend.uploaded_checksum());
    assert(second_snapshot.payload == second_payload);

    /*
     * Failed upload leaves no valid active snapshot.
     */
    assert(!backend.upload(second_payload, false));

    auto failed_snapshot = backend.snapshot();

    assert(failed_snapshot.generation == 2);
    assert(failed_snapshot.size == 0);
    assert(failed_snapshot.checksum == 0);
    assert(failed_snapshot.payload.empty());

    backend.reset();

    auto reset_snapshot = backend.snapshot();

    assert(reset_snapshot.generation == 0);
    assert(reset_snapshot.size == 0);
    assert(reset_snapshot.checksum == 0);
    assert(reset_snapshot.payload.empty());
}



/* CYCLE #42.29 — GPU upload snapshot validation contract */
static void cycle42_gpu_upload_snapshot_validation_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    /*
     * Empty snapshot is a valid "no upload" state.
     */
    auto empty = backend.snapshot();

    assert(empty.validate());

    std::vector<std::uint8_t> payload{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(payload));

    auto snapshot = backend.snapshot();

    assert(snapshot.validate());

    /*
     * Size mismatch invalidates the snapshot.
     */
    auto bad_size = snapshot;
    ++bad_size.size;

    assert(!bad_size.validate());

    /*
     * Checksum mismatch invalidates the snapshot.
     */
    auto bad_checksum = snapshot;
    ++bad_checksum.checksum;

    assert(!bad_checksum.validate());

    /*
     * Payload mismatch invalidates the snapshot.
     */
    auto bad_payload = snapshot;
    bad_payload.payload[0] ^= 0xFF;

    assert(!bad_payload.validate());

    /*
     * Generation zero is invalid for a non-empty snapshot.
     */
    auto bad_generation = snapshot;
    bad_generation.generation = 0;

    assert(!bad_generation.validate());

    /*
     * A valid snapshot remains independently verifiable.
     */
    auto copy = snapshot;

    assert(copy.validate());
    assert(copy.generation == snapshot.generation);
    assert(copy.size == snapshot.size);
    assert(copy.checksum == snapshot.checksum);
    assert(copy.payload == snapshot.payload);
}



/* CYCLE #42.30 — GPU upload snapshot generation consistency contract */
static void cycle42_gpu_upload_snapshot_generation_consistency_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    /*
     * Initial backend and snapshot must agree.
     */
    auto initial = backend.snapshot();

    assert(!backend.has_upload());
    assert(initial.generation == backend.upload_generation());
    assert(initial.generation == 0);
    assert(initial.size == 0);
    assert(initial.checksum == 0);
    assert(initial.payload.empty());
    assert(initial.validate());

    std::vector<std::uint8_t> first_payload{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(first_payload));

    auto first = backend.snapshot();

    assert(backend.has_upload());
    assert(backend.has_generation(first.generation));
    assert(first.generation == backend.upload_generation());
    assert(first.generation != 0);
    assert(first.size == backend.uploaded_size());
    assert(first.checksum == backend.uploaded_checksum());
    assert(first.payload == backend.uploaded_payload());
    assert(first.validate());

    /*
     * A second successful upload invalidates the old generation
     * as the currently active generation.
     */
    std::vector<std::uint8_t> second_payload{
        0x50, 0x60, 0x70
    };

    assert(backend.upload(second_payload));

    auto second = backend.snapshot();

    assert(second.generation == backend.upload_generation());
    assert(second.generation != first.generation);
    assert(!backend.has_generation(first.generation));
    assert(backend.has_generation(second.generation));
    assert(second.size == backend.uploaded_size());
    assert(second.checksum == backend.uploaded_checksum());
    assert(second.payload == backend.uploaded_payload());
    assert(second.validate());

    /*
     * Failed upload preserves the generation counter but removes
     * the active upload state.
     */
    assert(!backend.upload(second_payload, false));

    auto failed = backend.snapshot();

    assert(!backend.has_upload());
    assert(failed.generation == backend.upload_generation());
    assert(failed.generation == second.generation);
    assert(!backend.has_generation(second.generation));
    assert(failed.size == 0);
    assert(failed.checksum == 0);
    assert(failed.payload.empty());
    assert(failed.validate());

    /*
     * Reset must restore the complete initial state.
     */
    backend.reset();

    auto reset = backend.snapshot();

    assert(!backend.has_upload());
    assert(backend.upload_generation() == 0);
    assert(reset.generation == 0);
    assert(reset.size == 0);
    assert(reset.checksum == 0);
    assert(reset.payload.empty());
    assert(reset.validate());
}



/* CYCLE #42.31 — GPU upload snapshot immutability and isolation contract */
static void cycle42_gpu_upload_snapshot_immutability_isolation_contract() {
    using namespace pvr;

    GpuUploadBackend backend;

    std::vector<std::uint8_t> first_payload{
        0x10, 0x20, 0x30, 0x40
    };

    assert(backend.upload(first_payload));

    auto first = backend.snapshot();

    assert(first.validate());
    assert(first.payload == first_payload);

    /*
     * Mutating the original source after upload must not alter
     * the already captured snapshot.
     */
    first_payload[0] = 0xFF;
    first_payload.push_back(0xAA);

    assert(first.payload.size() == 4);
    assert(first.payload[0] == 0x10);
    assert(first.validate());

    /*
     * A second backend upload must not mutate the old snapshot.
     */
    std::vector<std::uint8_t> second_payload{
        0x50, 0x60, 0x70
    };

    assert(backend.upload(second_payload));

    auto second = backend.snapshot();

    assert(second.validate());
    assert(second.payload == second_payload);
    assert(second.generation != first.generation);

    assert(first.payload.size() == 4);
    assert(first.payload[0] == 0x10);
    assert(first.validate());

    /*
     * Mutating the second snapshot must not mutate backend state.
     */
    second.payload[0] = 0xEE;

    assert(second.payload[0] == 0xEE);
    assert(backend.uploaded_payload()[0] == 0x50);
    assert(backend.uploaded_size() == second_payload.size());
    assert(backend.uploaded_checksum() == second.checksum);
    assert(second.validate() == false);

    /*
     * Reset must not mutate either previously captured snapshot.
     */
    backend.reset();

    assert(!backend.has_upload());
    assert(backend.upload_generation() == 0);

    assert(first.payload.size() == 4);
    assert(first.payload[0] == 0x10);
    assert(first.validate());

    /*
     * The second snapshot remains an independent value after reset.
     */
    assert(second.payload.size() == 3);
    assert(second.payload[0] == 0xEE);
    assert(second.validate() == false);
}



void cycle42_gpu_upload_snapshot_restore_replay_contract() {
    using namespace pvr;

    std::vector<std::uint8_t> payload{
        0x21, 0x43, 0x65, 0x87, 0xA9
    };

    GpuUploadBackend source;
    assert(source.upload(payload));

    const GpuUploadSnapshot snapshot = source.snapshot();

    assert(snapshot.validate());
    assert(snapshot.generation != 0);
    assert(snapshot.size == payload.size());
    assert(snapshot.payload == payload);

    /*
     * A fresh backend must be able to restore the captured snapshot.
     */
    GpuUploadBackend restored;

    assert(restored.restore_snapshot(snapshot));

    const GpuUploadSnapshot replayed = restored.snapshot();

    assert(replayed.validate());
    assert(replayed.size == snapshot.size);
    assert(replayed.checksum == snapshot.checksum);
    assert(replayed.payload == snapshot.payload);

    /*
     * Restored backend owns an independent payload.
     */
    auto mutable_snapshot = snapshot;
    mutable_snapshot.payload[0] ^= 0xFF;

    assert(restored.uploaded_payload() == payload);
    assert(replayed.payload == payload);
}



void cycle42_gpu_upload_snapshot_restore_failure_isolation_contract() {
    using namespace pvr;

    std::vector<std::uint8_t> original_payload{
        0x11, 0x22, 0x33, 0x44
    };

    GpuUploadBackend backend;
    assert(backend.upload(original_payload));

    const auto before = backend.snapshot();

    assert(before.validate());
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == original_payload);

    /*
     * Construct a corrupted snapshot.
     */
    auto corrupted = before;
    corrupted.payload[0] ^= 0xFF;

    /*
     * The checksum still belongs to the original payload,
     * therefore validation must reject the snapshot.
     */
    assert(!corrupted.validate());

    /*
     * Restoring an invalid snapshot must fail.
     */
    assert(!backend.restore_snapshot(corrupted));

    /*
     * Existing backend state must remain untouched.
     */
    assert(backend.has_upload());
    assert(backend.uploaded_payload() == original_payload);
    assert(backend.uploaded_size() == original_payload.size());
    assert(backend.uploaded_checksum() == before.checksum);
    assert(backend.upload_generation() == before.generation);

    const auto after = backend.snapshot();

    assert(after.validate());
    assert(after.generation == before.generation);
    assert(after.size == before.size);
    assert(after.checksum == before.checksum);
    assert(after.payload == before.payload);
}



void cycle42_indirect_buffer_gpu_upload_contract() {
    using namespace pvr;

    std::vector<IndirectDrawCommand> commands{
        {
            6,
            1,
            0,
            0,
            0
        },
        {
            12,
            1,
            6,
            0,
            1
        }
    };

    IndirectBuffer buffer(commands);

    assert(!buffer.empty());
    assert(buffer.size() == 2);
    assert(buffer.validate(18));

    const auto expected_payload = buffer.upload_payload();

    GpuUploadBackend backend;

    /*
     * The backend should accept an IndirectBuffer directly.
     */
    assert(backend.upload(buffer));

    assert(backend.has_upload());
    assert(backend.uploaded_payload() == expected_payload);
    assert(backend.uploaded_size() == expected_payload.size());
    assert(backend.uploaded_checksum() != 0);

    /*
     * The backend owns its own uploaded payload.
     */
    assert(backend.uploaded_payload().data() !=
           expected_payload.data());

    /*
     * Mutating the original payload must not alter backend state.
     */
    auto mutable_payload = expected_payload;

    if (!mutable_payload.empty()) {
        mutable_payload[0] ^= 0xFF;
    }

    assert(backend.uploaded_payload() == expected_payload);
}


int main(){
 cycle42_indirect_buffer_gpu_upload_contract();
cycle42_gpu_upload_snapshot_restore_failure_isolation_contract();
 cycle42_gpu_upload_snapshot_restore_replay_contract();
 cycle42_gpu_upload_snapshot_immutability_isolation_contract();
 cycle42_gpu_upload_snapshot_generation_consistency_contract();
 cycle42_gpu_upload_snapshot_validation_contract();
 cycle42_gpu_upload_snapshot_contract();
 cycle42_gpu_upload_generation_aware_state_contract();
 cycle42_gpu_upload_generation_contract();
 cycle42_gpu_upload_payload_integrity_contract();
 cycle42_gpu_upload_payload_immutability_contract();
 cycle42_gpu_upload_payload_size_contract();
 cycle42_gpu_upload_backend_reset_contract();
 cycle42_gpu_upload_failure_contract();
 cycle42_gpu_upload_backend_contract();
 cycle42_indirect_buffer_upload_contract();
 cycle42_indirect_buffer_object_validation_contract();
 cycle42_indirect_buffer_payload_contract();
 cycle42_indirect_buffer_abstraction_contract();
 cycle42_indirect_buffer_validation_contract();
 cycle42_indirect_draw_buffer_contract();
 cycle42_draw_command_batching_contract();
 cycle42_draw_command_validation_contract();


 cycle42_compacted_draw_command_contract();
 cycle42_visible_cluster_compaction_contract();
 cycle42_mesh_cluster_batch_culling_determinism_contract();
 cycle42_mesh_cluster_batch_culling_contract();
 cycle42_mesh_cluster_culling_edge_cases_contract();
 cycle42_mesh_cluster_culling_contract();
 cycle42_mesh_cluster_spatial_separation_contract();
 cycle42_mesh_cluster_determinism_contract();
 cycle42_mesh_cluster_bounding_sphere_contract();
 cycle42_mesh_cluster_builder_contract();
 cycle42_mesh_cluster_contract();

 using namespace pvr;
 RepresentationManager rm;
 auto r=rm.request(42, RepresentationType::Coarse, 10.0);
 assert(r.id==42 && r.type==RepresentationType::Coarse);
 VoxelSection one(2,2,2); one.set(0,0,0,1);
 auto m1=GreedyMesher::build(one);
 assert(m1.quadCount==6);
 assert(m1.vertices.size()==24 && m1.indices.size()==36);
 VoxelSection pair(2,1,1); pair.set(0,0,0,1); pair.set(1,0,0,1);
 auto m2=GreedyMesher::build(pair);
 assert(m2.quadCount==6); // 10 faces collapse to 6 quads
 assert(m2.materials.size()==1 && m2.materials[0]==1);
 VoxelSection plane(2,1,2); for(unsigned x=0;x<2;++x) for(unsigned z=0;z<2;++z) plane.set(x,0,z,2);
 auto m3=GreedyMesher::build(plane);
 assert(m3.quadCount==6);

 VoxelSection square(2,1,2);
 square.set(0,0,0,3);
 square.set(1,0,0,3);
 square.set(0,0,1,3);
 square.set(1,0,1,3);
 auto m4=GreedyMesher::build(square);
 assert(m4.quadCount==6);
 assert(m4.materials.size()==1 && m4.materials[0]==3);

 VoxelSection material_boundary(2,1,2);
 material_boundary.set(0,0,0,4);
 material_boundary.set(0,0,1,4);
 material_boundary.set(1,0,0,5);
 material_boundary.set(1,0,1,5);
 auto m5=GreedyMesher::build(material_boundary);
 assert(m5.quadCount==10);
 assert(m5.materials.size()==2);

 VoxelSection l_shape(2,1,2);
 l_shape.set(0,0,0,6);
 l_shape.set(1,0,0,6);
 l_shape.set(0,0,1,6);
 auto m6=GreedyMesher::build(l_shape);
 assert(m6.quadCount==10);
 assert(m6.materials.size()==1 && m6.materials[0]==6);

 VoxelSection orientation(1,1,1);
 orientation.set(0,0,0,7);
 auto m7=GreedyMesher::build(orientation);

 bool has_pos_x=false, has_neg_x=false;
 bool has_pos_y=false, has_neg_y=false;
 bool has_pos_z=false, has_neg_z=false;

 for (const auto& v : m7.vertices) {
     if (v.nx > 0) has_pos_x=true;
     if (v.nx < 0) has_neg_x=true;
     if (v.ny > 0) has_pos_y=true;
     if (v.ny < 0) has_neg_y=true;
     if (v.nz > 0) has_pos_z=true;
     if (v.nz < 0) has_neg_z=true;
 }

 assert(has_pos_x && has_neg_x);
 assert(has_pos_y && has_neg_y);
 assert(has_pos_z && has_neg_z);

 VoxelSection quad_size(2,1,2);
 quad_size.set(0,0,0,8);
 quad_size.set(1,0,0,8);
 quad_size.set(0,0,1,8);
 quad_size.set(1,0,1,8);
 auto m8=GreedyMesher::build(quad_size);

 bool found_width_2=false;
 bool found_height_2=false;

 for (std::size_t i=0; i+3<m8.vertices.size(); i+=4) {
     const auto& a=m8.vertices[i];
     const auto& b=m8.vertices[i+1];
     const auto& c=m8.vertices[i+2];

     const float ab =
         std::abs(b.x-a.x) +
         std::abs(b.y-a.y) +
         std::abs(b.z-a.z);

     const float bc =
         std::abs(c.x-b.x) +
         std::abs(c.y-b.y) +
         std::abs(c.z-b.z);

     if (ab == 2.0f) found_width_2=true;
     if (bc == 2.0f) found_height_2=true;
 }

 assert(found_width_2 || found_height_2);
 std::cout<<"pvr_mesh_tests: PASS\n";
}
