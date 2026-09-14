#pragma once
#include <cstdint>
#include <unordered_map>
namespace pvr { enum class RepresentationType { Full, Reduced, Coarse, Abstract }; struct Representation { std::uint64_t id{}; RepresentationType type{}; double memoryCost{}; std::uint64_t version{}; }; class RepresentationManager { public: Representation request(std::uint64_t id, RepresentationType type, double memoryCost); const Representation* find(std::uint64_t id) const; private: std::unordered_map<std::uint64_t,Representation> cache_; }; }
