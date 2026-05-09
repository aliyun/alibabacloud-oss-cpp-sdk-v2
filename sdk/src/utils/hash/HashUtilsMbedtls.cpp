
#include "../Utils.h"
#include <cstring>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>

#if MBEDTLS_VERSION_NUMBER >= 0x03000000
#define oss2_sha256_starts(ctx, is224)        mbedtls_sha256_starts(ctx, is224)
#define oss2_sha256_update(ctx, input, ilen)  mbedtls_sha256_update(ctx, input, ilen)
#define oss2_sha256_finish(ctx, output)       mbedtls_sha256_finish(ctx, output)
#else
#define oss2_sha256_starts(ctx, is224)        mbedtls_sha256_starts_ret(ctx, is224)
#define oss2_sha256_update(ctx, input, ilen)  mbedtls_sha256_update_ret(ctx, input, ilen)
#define oss2_sha256_finish(ctx, output)       mbedtls_sha256_finish_ret(ctx, output)
#endif

namespace alibabacloud {
namespace oss2 {
namespace utils {

void HmacSha1(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[20]) {
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (info == nullptr) {
        std::memset(out, 0, 20);
        return;
    }
    mbedtls_md_hmac(info,
        static_cast<const unsigned char*>(key), numKeyBytes,
        static_cast<const unsigned char*>(data), numDataBytes,
        out);
}

void HmacSh256(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[32]) {
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (info == nullptr) {
        std::memset(out, 0, 32);
        return;
    }
    mbedtls_md_hmac(info,
        static_cast<const unsigned char*>(key), numKeyBytes,
        static_cast<const unsigned char*>(data), numDataBytes,
        out);
}

std::string HashSh256(const void* data, size_t numDataBytes) {
    unsigned char hash[32];

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    oss2_sha256_starts(&ctx, 0);
    oss2_sha256_update(&ctx, static_cast<const unsigned char*>(data), numDataBytes);
    oss2_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    static const char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(64);
    for (int i = 0; i < 32; i++) {
        result.push_back(hex[hash[i] >> 4]);
        result.push_back(hex[hash[i] & 0x0f]);
    }
    return result;
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
