
#include "alibabacloud/oss2/utils/PooledThreadExecutor.h"

namespace alibabacloud {
namespace oss2 {

PooledThreadExecutor::PooledThreadExecutor(size_t poolSize) {
    for (size_t i = 0; i < poolSize; ++i) {
        workers_.emplace_back(&PooledThreadExecutor::workerLoop, this);
    }
}

PooledThreadExecutor::~PooledThreadExecutor() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void PooledThreadExecutor::execute(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) {
            return;
        }
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void PooledThreadExecutor::workerLoop() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return stopped_ || !tasks_.empty(); });
            if (stopped_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        if (task) {
            task();
        }
    }
}

} // namespace oss2
} // namespace alibabacloud
