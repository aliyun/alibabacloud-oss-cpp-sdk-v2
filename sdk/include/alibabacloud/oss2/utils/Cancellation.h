
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>


namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API CancellationTokenSource;

/// Consumer-side handle for checking and waiting on cancellation.
/// Obtained via CancellationTokenSource::getToken(). Thread-safe; can be copied and shared.
class ALIBABACLOUD_OSS_API CancellationToken {
  public:
    /// Default-constructed token that can never be canceled.
    CancellationToken() : deadline_(nullptr) {}

    /// Returns true if this token is associated with a CancellationTokenSource.
    inline bool canBeCanceled() const {
        return deadline_ != nullptr;
    }

    /// Returns true if cancellation has been requested and the deadline has passed.
    inline bool isCanceled() const {
        return deadline_ != nullptr ? deadline_->load() <= std::chrono::steady_clock::now() : false;
    }

    /// Blocks up to @p timeout or until canceled, whichever comes first.
    /// If cancelAfter() was set before this call, the wait is shortened to min(timeout, timeToDeadline).
    /// If cancel() is called during the wait, the thread is woken immediately.
    /// Returns true if canceled, false if timed out.
    /// Returns true immediately if the source has been destroyed.
    bool waitFor(std::chrono::milliseconds timeout) const;

  private:
    friend class CancellationTokenSource;
    CancellationToken(std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline,
                      std::weak_ptr<CancellationTokenSource> source)
            : deadline_(deadline), source_(source) {}
    std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline_;
    std::weak_ptr<CancellationTokenSource> source_;
};


/// Producer-side control for requesting cancellation. Thread-safe.
///
/// @code
///   // Immediate cancellation — abort an in-flight request from another thread:
///   auto cts = CancellationTokenSource::create();
///   OperationOptions opts;
///   opts.cancellationToken = cts->getToken();
///   // ... launch request with opts ...
///   cts->cancel();   // wakes any waitFor() immediately
///
///   // Deadline-based cancellation — set a total request timeout:
///   auto cts = CancellationTokenSource::create();
///   cts->cancelAfter(std::chrono::seconds(30));   // request-level deadline
///   OperationOptions opts;
///   opts.cancellationToken = cts->getToken();
///   // ... launch request with opts ...
/// @endcode
class ALIBABACLOUD_OSS_API CancellationTokenSource : public std::enable_shared_from_this<CancellationTokenSource> {
  private:
    struct Key {};
    std::shared_ptr<std::atomic<std::chrono::steady_clock::time_point>> deadline_;

  public:
    explicit CancellationTokenSource(Key)
            : deadline_(std::make_shared<std::atomic<std::chrono::steady_clock::time_point>>(
                      (std::chrono::steady_clock::time_point::max)())) {}

    /// Creates a new CancellationTokenSource.
    static std::shared_ptr<CancellationTokenSource> create() {
        return std::make_shared<CancellationTokenSource>(Key{});
    }

    /// Cancels immediately and wakes any thread blocked in CancellationToken::waitFor().
    /// Use this to abort an in-flight operation from another thread.
    void cancel() {
        updateDeadline(std::chrono::steady_clock::now());
        std::lock_guard<std::mutex> lk(mu_);
        cv_.notify_all();
    }

    /// Sets a future cancellation deadline. The token becomes canceled when the
    /// deadline passes. Does NOT wake an in-progress waitFor(); the wait is
    /// shortened only if cancelAfter() is called before waitFor() enters its sleep.
    /// Use cancel() for immediate interruption of an active wait.
    void cancelAfter(std::chrono::milliseconds after) {
        updateDeadline(std::chrono::steady_clock::now() + after);
    }

    /// @copydoc cancelAfter(std::chrono::milliseconds)
    void cancelAfter(std::chrono::system_clock::time_point timepoint) {
        updateDeadline(std::chrono::steady_clock::now() + (timepoint - std::chrono::system_clock::now()));
    }

    /// Returns a CancellationToken bound to this source.
    CancellationToken getToken() {
        return CancellationToken(deadline_, shared_from_this());
    }

    /// Returns the current cancellation deadline (time_point::max if not set).
    std::chrono::steady_clock::time_point getDeadline() {
        return deadline_->load();
    }

  private:
    friend class CancellationToken;
    std::mutex mu_;
    std::condition_variable cv_;

    void updateDeadline(std::chrono::steady_clock::time_point timepoint) {
        auto current = deadline_->load();
        while (timepoint < current) {
            if (deadline_->compare_exchange_weak(current, timepoint)) {
                break;
            }
        }
    }
};

} // namespace oss2
} // namespace alibabacloud