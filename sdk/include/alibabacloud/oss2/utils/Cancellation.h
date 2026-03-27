
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <atomic>
#include <chrono>
#include <memory>


namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API CancellationTokenSource;

class ALIBABACLOUD_OSS_API CancellationToken {
  public:
    CancellationToken() : deadline_(nullptr) {}

    inline bool canBeCanceled() const {
        return deadline_ != nullptr;
    }

    inline bool isCanceled() const {
        return deadline_ != nullptr ? deadline_->load() <= std::chrono::steady_clock::now() : false;
    }

  private:
    friend class CancellationTokenSource;
    CancellationToken(std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline,
                      std::weak_ptr<CancellationTokenSource> source)
            : deadline_(deadline), source_(source) {}
    std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline_;
    std::weak_ptr<CancellationTokenSource> source_;
};


class ALIBABACLOUD_OSS_API CancellationTokenSource : public std::enable_shared_from_this<CancellationTokenSource> {
  private:
    struct Key {};
    std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline_;

  public:
    explicit CancellationTokenSource(Key)
            : deadline_(std::make_shared<std::atomic<std::chrono::steady_clock::time_point>>(
                      std::chrono::steady_clock::time_point::max())) {}

    static std::shared_ptr<CancellationTokenSource> create() {
        return std::make_shared<CancellationTokenSource>(Key{});
    }

    void cancel() {
        updateDealine(std::chrono::steady_clock::now());
    }

    void cancelAfter(std::chrono::milliseconds after) {
        updateDealine(std::chrono::steady_clock::now() + after);
    }

    void cancelAfter(std::chrono::system_clock::time_point timepoint) {
        updateDealine(std::chrono::steady_clock::now() + (timepoint - std::chrono::system_clock::now()));
    }

    CancellationToken getToken() {
        return CancellationToken(deadline_, shared_from_this());
    }

    std::chrono::steady_clock::time_point getDeadline() {
        return deadline_->load();
    }

  private:
    void updateDealine(std::chrono::steady_clock::time_point timepoint) {
        if (timepoint < deadline_->load()) {
            deadline_->store(timepoint);
        }
    }
};

} // namespace oss2
} // namespace alibabacloud