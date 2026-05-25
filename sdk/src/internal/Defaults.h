#pragma once

#include "alibabacloud/oss2/Types.h"

#include <chrono>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace internal {
namespace defaults {

constexpr std::string_view PRODUCT = "oss";
constexpr std::string_view HTTP_SCHEME = "https";
constexpr bool DISABLE_SSL = false;

// defaults for retryer
constexpr long MAX_ATTEMPTS = 3;
constexpr std::chrono::milliseconds BASE_DELAY = std::chrono::milliseconds(200);
constexpr std::chrono::milliseconds MAX_BACKOFF = std::chrono::milliseconds(20000);

constexpr int FEATURE_FLAGS =
    static_cast<int>(FeatureFlagsType::CorrectClockSkew) |
    static_cast<int>(FeatureFlagsType::AutoDetectMimeType) |
    static_cast<int>(FeatureFlagsType::EnableCRC64CheckUpload) |
    static_cast<int>(FeatureFlagsType::EnableCRC64CheckDownload);

} // namespace defaults
} // namespace internal
} // namespace oss2
} // namespace alibabacloud