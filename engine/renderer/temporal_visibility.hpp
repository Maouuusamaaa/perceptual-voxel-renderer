#pragma once
#include <cstddef>
#include <cstdint>
#include <unordered_map>
namespace pvr {
enum class VisibilityConfidence { Low, Medium, High };
class TemporalVisibilityCache {
public:
    explicit TemporalVisibilityCache(std::size_t ttl_frames=3): ttl_frames_(ttl_frames) {}
    void observe(std::uint64_t id, bool visible);
    void begin_frame();
    VisibilityConfidence confidence(std::uint64_t id) const;
    bool contains(std::uint64_t id) const;
private:
    struct Entry { std::uint32_t visible_streak{}; std::uint32_t miss_streak{}; std::uint64_t last_frame{}; };
    std::unordered_map<std::uint64_t,Entry> entries_;
    std::uint64_t frame_{0};
    std::size_t ttl_frames_;
};
}
