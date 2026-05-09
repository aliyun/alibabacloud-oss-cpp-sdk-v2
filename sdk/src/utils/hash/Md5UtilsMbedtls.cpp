
#include "../Utils.h"
#include <mbedtls/md5.h>
#include <mbedtls/version.h>

#if MBEDTLS_VERSION_NUMBER >= 0x03000000
#define oss2_md5_starts(ctx)              mbedtls_md5_starts(ctx)
#define oss2_md5_update(ctx, input, ilen) mbedtls_md5_update(ctx, input, ilen)
#define oss2_md5_finish(ctx, output)      mbedtls_md5_finish(ctx, output)
#else
#define oss2_md5_starts(ctx)              mbedtls_md5_starts_ret(ctx)
#define oss2_md5_update(ctx, input, ilen) mbedtls_md5_update_ret(ctx, input, ilen)
#define oss2_md5_finish(ctx, output)      mbedtls_md5_finish_ret(ctx, output)
#endif

namespace alibabacloud {
namespace oss2 {
namespace utils {

std::string CalcContentMD5(const std::string& data) {
    return CalcContentMD5(data.data(), data.size());
}

std::string CalcContentMD5(const char* data, size_t size) {
    unsigned char md_value[16];

    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    oss2_md5_starts(&ctx);
    oss2_md5_update(&ctx, reinterpret_cast<const unsigned char*>(data), size);
    oss2_md5_finish(&ctx, md_value);
    mbedtls_md5_free(&ctx);

    return Base64Encode(reinterpret_cast<const std::byte*>(md_value), 16);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
