#include "core/job_system.hpp"
#include <algorithm>
#include <stdexcept>
namespace pvr {
JobSystem::JobSystem(std::size_t workerCount) {
    if (workerCount == 0) throw std::invalid_argument("job system requires at least one worker");
    workers_.reserve(workerCount);
    for (std::size_t i=0;i<workerCount;++i) workers_.emplace_back(&JobSystem::worker, this);
}
JobSystem::~JobSystem() {
    { std::lock_guard lock(mutex_); stopping_=true; }
    cv_.notify_all();
    for (auto& w:workers_) if(w.joinable()) w.join();
}
void JobSystem::worker() {
    for (;;) {
        std::function<void()> job;
        { std::unique_lock lock(mutex_); cv_.wait(lock, [this]{ return stopping_ || !jobs_.empty(); });
          if (stopping_ && jobs_.empty()) return;
          job=std::move(jobs_.front()); jobs_.pop(); }
        job();
    }
}
}
