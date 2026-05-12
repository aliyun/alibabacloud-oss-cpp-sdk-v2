
#include "alibabacloud/oss2/utils/DefaultExecutor.h"


namespace alibabacloud {
namespace oss2 {

DefaultExecutor::~DefaultExecutor() {
    std::unordered_map<std::thread::id, std::thread> threads;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        threads.swap(threads_);
    }
    for (auto& [id, t] : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void DefaultExecutor::execute(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_) {
        return;
    }
    std::thread t([this, task = std::move(task)] {
        task();
        detach(std::this_thread::get_id());
    });
    threads_.emplace(t.get_id(), std::move(t));
}

void DefaultExecutor::detach(std::thread::id id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = threads_.find(id);
    if (it != threads_.end()) {
        it->second.detach();
        threads_.erase(it);
    }
}
} // namespace oss2
} // namespace alibabacloud
