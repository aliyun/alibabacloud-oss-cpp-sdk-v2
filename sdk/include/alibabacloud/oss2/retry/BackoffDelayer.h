
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <chrono>
#include <system_error>

namespace alibabacloud {
namespace oss2 {

/**
 * A interface class defining the backoff delay strategy for retries.
 */
class ALIBABACLOUD_OSS_API BackoffDelayer {
  public:
    /**
     * Calculates the delay in milliseconds before the next retry based on the current attempt count and error.
     */
    virtual std::chrono::milliseconds backoffDelay(long attempt, const std::error_code& error) const = 0;
    virtual ~BackoffDelayer() = default;
};

/**
 * Implements a retry backoff strategy with a fixed delay.
 */
class ALIBABACLOUD_OSS_API FixedDelayBackoff : public BackoffDelayer {
  public:
    FixedDelayBackoff(std::chrono::milliseconds delay) : delay_(delay) {}

    std::chrono::milliseconds backoffDelay(long attempt, const std::error_code& error) const override;

  private:
    std::chrono::milliseconds delay_;
};

/**
 * Implements an exponential backoff strategy with equal jitter for retry delays.
 * delay = min(2 ^ attempts * baseDealy, maxBackoff)
 * delay/2 + [0.0, 1.0) * delay/2
 */
class ALIBABACLOUD_OSS_API EqualJitterBackoff : public BackoffDelayer {
  public:
    EqualJitterBackoff(std::chrono::milliseconds baseDelay, std::chrono::milliseconds maxBackoff)
        : baseDelay_(baseDelay), maxBackoff_(maxBackoff) {}

    std::chrono::milliseconds backoffDelay(long attempt, const std::error_code& error) const override;

  private:
    std::chrono::milliseconds baseDelay_;
    std::chrono::milliseconds maxBackoff_;
};

/**
 * Implements a full jitter backoff strategy for retry delays.
 * [0.0, 1.0) * min(2 ^ attempts * baseDealy, maxBackoff)
 */
class ALIBABACLOUD_OSS_API FullJitterBackoff : public BackoffDelayer {
  public:
    FullJitterBackoff(std::chrono::milliseconds baseDelay, std::chrono::milliseconds maxBackoff)
        : baseDelay_(baseDelay), maxBackoff_(maxBackoff) {}

    std::chrono::milliseconds backoffDelay(long attempt, const std::error_code& error) const override;

  private:
    std::chrono::milliseconds baseDelay_;
    std::chrono::milliseconds maxBackoff_;
};


} // namespace oss2
} // namespace alibabacloud