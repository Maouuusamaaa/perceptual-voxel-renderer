#pragma once
#include <condition_variable>
#include <cstddef>
#include <future>
#include <functional>
#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace pvr {
class JobSystem {
public:
    explicit JobSystem(std::size_t workerCount);
    ~JobSystem();
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    template<class F> auto submit(F&& fn) -> std::future<std::invoke_result_t<F>> {
        using R = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(fn));
        auto future = task->get_future();
        {
            std::lock_guard lock(mutex_);
            jobs_.emplace([task] { (*task)(); });
        }
        cv_.notify_one();
        return future;
    }
private:
    void worker();
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> jobs_;
    std::vector<std::thread> workers_;
    bool stopping_{false};
};
struct VersionedJob { static bool isCurrent(std::uint64_t inputVersion, std::uint64_t currentVersion) noexcept { return inputVersion == currentVersion; } };
}
