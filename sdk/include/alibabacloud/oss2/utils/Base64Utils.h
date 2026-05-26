#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <cstddef>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace utils {

ALIBABACLOUD_OSS_API std::string Base64Encode(const std::string& src);
ALIBABACLOUD_OSS_API std::string Base64Encode(const std::byte* src, std::size_t len);
ALIBABACLOUD_OSS_API std::string Base64Decode(const std::string& src);

inline std::string base64Encode(const std::string& src) {
    return Base64Encode(src);
}

inline std::string base64Encode(const std::byte* src, std::size_t len) {
    return Base64Encode(src, len);
}

inline std::string base64Decode(const std::string& src) {
    return Base64Decode(src);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
