#pragma once

#include "alibabacloud/oss2/utils/Executor.h"

#include <chrono>

namespace alibabacloud {
namespace oss2 {

/// Abstract base class for executors that support delayed task scheduling.
/// Extends Executor with a schedule(delay, task) method.
/// Subclass this to integrate custom async frameworks (e.g., Asio, libuv).
class ALIBABACLOUD_OSS_API ScheduledExecutor : public Executor {
  public:
    ~ScheduledExecutor() override = default;

    virtual void schedule(std::chrono::milliseconds delay, std::function<void()> task) = 0;

    void execute(std::function<void()> task) override {
        schedule(std::chrono::milliseconds(0), std::move(task));
    }
};

} // namespace oss2
} // namespace alibabacloud
