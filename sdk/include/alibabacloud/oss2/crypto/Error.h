#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <system_error>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

enum class CryptoErrorCode {
    EncryptFailed = 1,
    DecryptFailed,
    InvalidKey,
    RandomGenerationFailed,
    EmptyKeyOrIV,
    InvalidKeyOrIV,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(CryptoErrorCode e);

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud

template<>
struct std::is_error_code_enum<alibabacloud::oss2::crypto::CryptoErrorCode> : std::true_type {};
