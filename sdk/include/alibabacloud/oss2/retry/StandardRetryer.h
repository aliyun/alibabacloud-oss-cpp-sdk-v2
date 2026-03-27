
#pragma once

#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/ErrorRetryable.h"
#include "alibabacloud/oss2/retry/Retryer.h"


#include <memory>
#include <vector>


namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API StandardRetryer final : public Retryer {
  public:
    StandardRetryer(long maxAttempts, std::unique_ptr<BackoffDelayer> backoffDelayer = nullptr,
                    std::unique_ptr<ErrorRetryable> errorRetryable = nullptr);

    long getMaxAttempts() const override {
        return maxAttempts_;
    }

    std::chrono::milliseconds calcDelayTime(const std::error_code& error, long attempt) const override {
        return backoffDelayer_->backoffDelay(attempt, error);
    }

    bool isErrorRetryable(const std::error_code& error) const override {
        return errorRetryable_->isErrorRetryable(error);
    }

    std::string getName() const override {
        return "StandardRetryer";
    }

  private:
    long maxAttempts_;
    std::unique_ptr<BackoffDelayer> backoffDelayer_;
    std::unique_ptr<ErrorRetryable> errorRetryable_;
};


} // namespace oss2
} // namespace alibabacloud
