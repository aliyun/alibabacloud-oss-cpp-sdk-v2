
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <chrono>
#include <system_error>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API Retryer {
  public:
    virtual long getMaxAttempts() const = 0;
    virtual std::chrono::milliseconds calcDelayTime(const std::error_code&, long) const = 0;
    virtual bool isErrorRetryable(const std::error_code&) const = 0;

    virtual ~Retryer() = default;

    virtual std::string getName() const {
        return "";
    }
};

class ALIBABACLOUD_OSS_API NopRetryer final : public Retryer {
  public:
    long getMaxAttempts() const override {
        return 1L;
    }
    std::chrono::milliseconds calcDelayTime(const std::error_code&, long) const override {
        return std::chrono::milliseconds(0);
    }
    bool isErrorRetryable(const std::error_code&) const override {
        return false;
    }
};

} // namespace oss2
} // namespace alibabacloud
