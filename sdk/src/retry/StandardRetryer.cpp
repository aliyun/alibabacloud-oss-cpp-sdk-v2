
#include "alibabacloud/oss2/retry/StandardRetryer.h"

namespace alibabacloud {
namespace oss2 {

StandardRetryer::StandardRetryer(long maxAttempts, std::unique_ptr<BackoffDelayer> backoffDelayer,
                                 std::unique_ptr<ErrorRetryable> errorRetryable)
        : maxAttempts_(maxAttempts), backoffDelayer_(std::move(backoffDelayer)),
          errorRetryable_(std::move(errorRetryable)) {
    if (maxAttempts_ <= 0) {
        maxAttempts_ = 1;
    }

    if (backoffDelayer_ == nullptr) {
        backoffDelayer_ =
                std::make_unique<FullJitterBackoff>(std::chrono::milliseconds(200), std::chrono::milliseconds(20000));
    }

    if (errorRetryable_ == nullptr) {
        errorRetryable_ = std::make_unique<DefaultErrorRetryable>();
    }
}

} // namespace oss2
} // namespace alibabacloud