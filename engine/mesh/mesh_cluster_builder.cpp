#include "mesh/mesh_cluster_builder.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace pvr {

std::vector<MeshCluster> MeshClusterBuilder::build(
    const Mesh& mesh,
    std::uint32_t quads_per_cluster) const {

    if (quads_per_cluster == 0) {
        throw std::invalid_argument("quads_per_cluster must be greater than zero");
    }

    if (mesh.quadCount == 0) {
        return {};
    }

    constexpr std::uint32_t indices_per_quad = 6;
    constexpr std::uint32_t vertices_per_quad = 4;

    const std::uint32_t cluster_count =
        (mesh.quadCount + quads_per_cluster - 1) /
        quads_per_cluster;

    std::vector<MeshCluster> clusters;
    clusters.reserve(cluster_count);

    for (std::uint32_t cluster_id = 0;
         cluster_id < cluster_count;
         ++cluster_id) {

        const std::uint32_t first_quad =
            cluster_id * quads_per_cluster;

        const std::uint32_t remaining =
            mesh.quadCount - first_quad;

        const std::uint32_t quad_count =
            std::min(quads_per_cluster, remaining);

        MeshCluster cluster{};

        cluster.first_index =
            first_quad * indices_per_quad;

        cluster.index_count =
            quad_count * indices_per_quad;

        cluster.first_vertex =
            first_quad * vertices_per_quad;

        cluster.vertex_count =
            quad_count * vertices_per_quad;

        const std::size_t vertex_begin =
            static_cast<std::size_t>(cluster.first_vertex);

        const std::size_t vertex_end =
            vertex_begin +
            static_cast<std::size_t>(cluster.vertex_count);

        if (vertex_end > mesh.vertices.size()) {
            throw std::runtime_error(
                "Mesh vertex data is inconsistent with quadCount");
        }

        if (cluster.first_index +
            cluster.index_count > mesh.indices.size()) {
            throw std::runtime_error(
                "Mesh index data is inconsistent with quadCount");
        }

        if (vertex_begin < mesh.vertices.size()) {
            cluster.material =
                mesh.vertices[vertex_begin].material;
        }

        if (cluster.vertex_count > 0) {
            const Vertex& first =
                mesh.vertices[vertex_begin];

            cluster.min_x = cluster.max_x = first.x;
            cluster.min_y = cluster.max_y = first.y;
            cluster.min_z = cluster.max_z = first.z;

            for (std::size_t i = vertex_begin;
                 i < vertex_end;
                 ++i) {

                const Vertex& vertex = mesh.vertices[i];

                cluster.min_x = std::min(cluster.min_x, vertex.x);
                cluster.min_y = std::min(cluster.min_y, vertex.y);
                cluster.min_z = std::min(cluster.min_z, vertex.z);

                cluster.max_x = std::max(cluster.max_x, vertex.x);
                cluster.max_y = std::max(cluster.max_y, vertex.y);
                cluster.max_z = std::max(cluster.max_z, vertex.z);
            }

            cluster.center_x =
                cluster.min_x +
                (cluster.max_x - cluster.min_x) * 0.5f;

            cluster.center_y =
                cluster.min_y +
                (cluster.max_y - cluster.min_y) * 0.5f;

            cluster.center_z =
                cluster.min_z +
                (cluster.max_z - cluster.min_z) * 0.5f;

            float radius_squared = 0.0f;

            for (std::size_t i = vertex_begin;
                 i < vertex_end;
                 ++i) {

                const Vertex& vertex = mesh.vertices[i];

                const float dx =
                    vertex.x - cluster.center_x;
                const float dy =
                    vertex.y - cluster.center_y;
                const float dz =
                    vertex.z - cluster.center_z;

                const float distance_squared =
                    dx * dx + dy * dy + dz * dz;

                radius_squared =
                    std::max(radius_squared, distance_squared);
            }

            cluster.radius = std::sqrt(radius_squared);
        }

        clusters.push_back(cluster);
    }

    return clusters;
}

}
