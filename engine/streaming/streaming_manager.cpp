#include "streaming/streaming_manager.hpp"
#include <stdexcept>
namespace pvr {
void StreamingManager::enqueue(StreamingRequest request) { queue_.push(request); }
StreamingRequest StreamingManager::popNext() { if (queue_.empty()) throw std::out_of_range("streaming queue empty"); auto r=queue_.top(); queue_.pop(); return r; }
bool StreamingManager::empty() const noexcept { return queue_.empty(); }
std::size_t StreamingManager::size() const noexcept { return queue_.size(); }
}
