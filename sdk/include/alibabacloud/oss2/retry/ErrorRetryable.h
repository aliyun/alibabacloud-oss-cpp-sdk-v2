
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <chrono>
#include <system_error>

namespace alibabacloud {
namespace oss2 {

/**
 * A interface class used to determine whether a given error is retryable.
 */
class ALIBABACLOUD_OSS_API ErrorRetryable {
  public:
    virtual bool isErrorRetryable(const std::error_code& error) const = 0;
    virtual ~ErrorRetryable() = default;
};

/**
 * Helper class to determine if a error is retryable.
 * Includes
 * 1. client-side error, HTTP status code
 * 2. a service error is retryable based on its HTTP status code
 * 3. a service error is retryable based on its error code
 */
class ALIBABACLOUD_OSS_API DefaultErrorRetryable : public ErrorRetryable {
  public:
    bool isErrorRetryable(const std::error_code& error) const override;
};


} // namespace oss2
} // namespace alibabacloud