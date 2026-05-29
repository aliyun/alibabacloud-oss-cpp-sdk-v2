#pragma once

#include "alibabacloud/oss2/utils/ScheduledExecutor.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace alibabacloud {
namespace oss2 {

class DefaultScheduledExecutor : public ScheduledExecutor {
  public:
    DefaultScheduledExecutor();
    ~DefaultScheduledExecutor() override;

    DefaultScheduledExecutor(const DefaultScheduledExecutor&) = delete;
    DefaultScheduledExecutor& operator=(const DefaultScheduledExecutor&) = delete;

    void schedule(std::chrono::milliseconds delay, std::function<void()> task) override;

  private:
    struct Entry {
        std::chrono::steady_clock::time_point deadline;
        std::function<void()> task;
        bool operator>(const Entry& o) const {
            return deadline > o.deadline;
        }
    };

    struct Impl {
        std::mutex mutex;
        std::condition_variable cv;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;
        bool stop{false};
        std::thread thread;
    };

    static void run(std::shared_ptr<Impl> impl);

    std::shared_ptr<Impl> impl_;
};

} // namespace oss2
} // namespace alibabacloud
