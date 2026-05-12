#pragma once

#include "alibabacloud/oss2/utils/Executor.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace alibabacloud {
namespace oss2 {

/// Thread-pool executor that maintains a fixed number of worker threads.
/// Tasks are queued and dispatched to idle workers via a condition variable.
class ALIBABACLOUD_OSS_API PooledThreadExecutor : public Executor {
  public:
    explicit PooledThreadExecutor(size_t poolSize);
    ~PooledThreadExecutor() override;

    PooledThreadExecutor(const PooledThreadExecutor&) = delete;
    PooledThreadExecutor& operator=(const PooledThreadExecutor&) = delete;
    PooledThreadExecutor(PooledThreadExecutor&&) = delete;
    PooledThreadExecutor& operator=(PooledThreadExecutor&&) = delete;

    void execute(std::function<void()> task) override;

  private:
    void workerLoop();

    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

} // namespace oss2
} // namespace alibabacloud
