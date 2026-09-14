#include "spatial/spatial_hierarchy.hpp"
#include <stdexcept>
namespace pvr {
SpatialHierarchy::SpatialHierarchy() { nodes_.emplace(0, SpatialNode{0,0,NodeType::World,{}}); }
NodeId SpatialHierarchy::addRegion(std::int64_t x,std::int64_t y,std::int64_t z,double size){
    NodeId id=nextId_++; SpatialNode n{id,0,NodeType::Region,{double(x),double(y),double(z),double(x)+size,double(y)+size,double(z)+size}}; nodes_[0].children.push_back(id); nodes_.emplace(id,std::move(n)); return id;
}
NodeId SpatialHierarchy::addChild(NodeId p,NodeType t){ if(!nodes_.contains(p)) throw std::out_of_range("parent node"); NodeId id=nextId_++; SpatialNode n{id,p,t,{}}; nodes_[p].children.push_back(id); nodes_.emplace(id,std::move(n)); return id; }
NodeId SpatialHierarchy::parent(NodeId id) const { return nodes_.at(id).parent; }
const std::vector<NodeId>& SpatialHierarchy::children(NodeId id) const { return nodes_.at(id).children; }
}
