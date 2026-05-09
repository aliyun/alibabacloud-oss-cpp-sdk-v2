
#include "../Utils.h"
#include <openssl/evp.h>

namespace alibabacloud {
namespace oss2 {
namespace utils {

std::string CalcContentMD5(const std::string& data) {
    return CalcContentMD5(data.data(), data.size());
}

std::string CalcContentMD5(const char* data, size_t size) {
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
    EVP_DigestUpdate(ctx, data, size);
    EVP_DigestFinal_ex(ctx, md_value, &md_len);
    EVP_MD_CTX_free(ctx);

    return Base64Encode(reinterpret_cast<const std::byte*>(md_value), md_len);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
