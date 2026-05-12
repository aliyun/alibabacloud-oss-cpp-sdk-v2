#pragma once

#include "alibabacloud/oss2/utils/Executor.h"

#include <mutex>
#include <thread>
#include <unordered_map>

namespace alibabacloud {
namespace oss2 {

/// Default executor that spawns a new thread for each submitted task.
/// Completed threads are detached; remaining threads are joined on destruction.
class ALIBABACLOUD_OSS_API DefaultExecutor : public Executor {
  public:
    DefaultExecutor() = default;
    ~DefaultExecutor() override;
    void execute(std::function<void()> task) override;

  private:
    void detach(std::thread::id id);
    mutable std::mutex mutex_;
    std::unordered_map<std::thread::id, std::thread> threads_;
    bool stopped_ = false;
};
} // namespace oss2
} // namespace alibabacloud
