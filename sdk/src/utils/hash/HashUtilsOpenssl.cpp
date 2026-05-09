
#include "../Utils.h"
#include <cstring>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace alibabacloud {
namespace oss2 {
namespace utils {

void HmacSha1(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[20]) {
    unsigned int outLen = 20;
    unsigned char* ret = HMAC(EVP_sha1(),
        key, static_cast<int>(numKeyBytes),
        static_cast<const unsigned char*>(data), numDataBytes,
        out, &outLen);
    if (ret == nullptr) {
        std::memset(out, 0, 20);
    }
}

void HmacSh256(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[32]) {
    unsigned int outLen = 32;
    unsigned char* ret = HMAC(EVP_sha256(),
        key, static_cast<int>(numKeyBytes),
        static_cast<const unsigned char*>(data), numDataBytes,
        out, &outLen);
    if (ret == nullptr) {
        std::memset(out, 0, 32);
    }
}

std::string HashSh256(const void* data, size_t numDataBytes) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        return std::string(64, '0');
    }
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data, numDataBytes) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &hashLen) != 1) {
        EVP_MD_CTX_free(ctx);
        return std::string(64, '0');
    }
    EVP_MD_CTX_free(ctx);

    static const char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(hashLen * 2);
    for (unsigned int i = 0; i < hashLen; i++) {
        result.push_back(hex[hash[i] >> 4]);
        result.push_back(hex[hash[i] & 0x0f]);
    }
    return result;
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
