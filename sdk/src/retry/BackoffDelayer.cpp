
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "src/utils/Utils.h"

#include <iostream>
#include <random>

namespace alibabacloud {
namespace oss2 {

constexpr long RETRIES_ATTEMPTED_CEILING = 31; // std::numeric_limits<long>::digits - 1;

std::chrono::milliseconds EqualJitterBackoff::backoffDelay(long attempt, const std::error_code&) const {
    attempt = std::min(attempt, RETRIES_ATTEMPTED_CEILING);
    auto ceil = std::min(baseDelay_ * (static_cast<int64_t>(1) << attempt), maxBackoff_);
    auto factor = static_cast<double>(utils::GetRandomValue()) / std::mt19937::max();
    auto delay =
            ceil / 2 +
            std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(
                    (std::chrono::duration<double, std::chrono::milliseconds::period>(ceil / 2) * factor).count()));
    return delay;
}

std::chrono::milliseconds FixedDelayBackoff::backoffDelay(long, const std::error_code&) const {
    return delay_;
}

std::chrono::milliseconds FullJitterBackoff::backoffDelay(long attempt, const std::error_code&) const {
    attempt = std::min(attempt, RETRIES_ATTEMPTED_CEILING);
    auto ceil = std::min(baseDelay_ * (static_cast<int64_t>(1) << attempt), maxBackoff_);
    auto factor = static_cast<double>(utils::GetRandomValue()) / std::mt19937::max();
    return std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(
            (std::chrono::duration<double, std::chrono::milliseconds::period>(ceil) * factor).count()));
}

} // namespace oss2
} // namespace alibabacloud