#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace pvr {
enum class NodeType { World, Region, Cluster, Chunk, MeshCluster };
using NodeId = std::uint64_t;
struct Bounds { double minX{}, minY{}, minZ{}, maxX{}, maxY{}, maxZ{}; };
struct SpatialNode { NodeId id{}; NodeId parent{}; NodeType type{}; Bounds bounds{}; std::vector<NodeId> children; };
class SpatialHierarchy {
public:
    SpatialHierarchy();
    NodeId addRegion(std::int64_t x, std::int64_t y, std::int64_t z, double size);
    NodeId addChild(NodeId parent, NodeType type);
    NodeId parent(NodeId id) const;
    const std::vector<NodeId>& children(NodeId id) const;
private:
    NodeId nextId_ = 1;
    std::unordered_map<NodeId, SpatialNode> nodes_;
};
}
