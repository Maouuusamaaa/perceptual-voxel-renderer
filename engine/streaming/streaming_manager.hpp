#pragma once
#include "representation/representation.hpp"
#include <cstdint>
#include <queue>

namespace pvr {
enum class PredictionConfidence { Low, Medium, High };
enum class StreamingReason { Visible, Prediction, Distance, Importance, MemoryPressure };
struct StreamingRequest {
    std::uint64_t spatialId{};
    RepresentationType desiredRepresentation{RepresentationType::Coarse};
    float priority{};
    PredictionConfidence confidence{PredictionConfidence::Low};
    StreamingReason reason{StreamingReason::Distance};
    std::uint64_t deadline{};
};
class StreamingManager {
public:
    void enqueue(StreamingRequest request);
    StreamingRequest popNext();
    bool empty() const noexcept;
    std::size_t size() const noexcept;
private:
    struct Compare { bool operator()(const StreamingRequest& a, const StreamingRequest& b) const noexcept { return a.priority < b.priority; } };
    std::priority_queue<StreamingRequest, std::vector<StreamingRequest>, Compare> queue_;
};
}
