#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <cstddef>
#include <cstdint>

namespace alibabacloud {
namespace oss2 {
namespace utils {

ALIBABACLOUD_OSS_API uint64_t CalcCRC64(uint64_t crc, const void* buf, size_t len);
ALIBABACLOUD_OSS_API uint64_t CombineCRC64(uint64_t crc1, uint64_t crc2, uintmax_t len2);

inline uint64_t calcCRC64(uint64_t crc, const void* buf, size_t len) {
    return CalcCRC64(crc, buf, len);
}

inline uint64_t combineCRC64(uint64_t crc1, uint64_t crc2, uintmax_t len2) {
    return CombineCRC64(crc1, crc2, len2);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
