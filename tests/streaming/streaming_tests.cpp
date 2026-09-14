#include "streaming/streaming_manager.hpp"
#include "core/job_system.hpp"
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    using namespace pvr;
    StreamingManager streaming;
    streaming.enqueue({1, RepresentationType::Coarse, 2.0f, PredictionConfidence::Medium, StreamingReason::Prediction, 20});
    streaming.enqueue({2, RepresentationType::Full, 8.0f, PredictionConfidence::High, StreamingReason::Visible, 10});
    streaming.enqueue({3, RepresentationType::Reduced, 4.0f, PredictionConfidence::Low, StreamingReason::Distance, 30});
    assert(streaming.popNext().spatialId == 2);
    assert(streaming.popNext().spatialId == 3);
    assert(streaming.popNext().spatialId == 1);
    assert(streaming.empty());

    JobSystem jobs(2);
    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 4; ++i) futures.push_back(jobs.submit([i] { return i * i; }));
    int total = 0;
    for (auto& f : futures) total += f.get();
    assert(total == 30);

    assert(VersionedJob::isCurrent(7, 7));
    assert(!VersionedJob::isCurrent(6, 7));
    std::cout << "pvr_streaming_tests: PASS\n";
}
