#include "../Aes256Utils.h"

#include <cstring>
#include <mbedtls/aes.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/platform_util.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

struct AesCtrContext {
    mbedtls_aes_context aes;
    unsigned char nonce_counter[16];
    unsigned char stream_block[16];
    size_t nc_off;
};

AesCtrCipher::AesCtrCipher(const std::string& key, const std::string& iv) : ctx_(nullptr) {
    if (key.size() != 16 && key.size() != 32) {
        return;
    }
    if (iv.size() < 16) {
        return;
    }

    auto* ctx = new (std::nothrow) AesCtrContext;
    if (!ctx) {
        return;
    }

    mbedtls_aes_init(&ctx->aes);
    if (mbedtls_aes_setkey_enc(&ctx->aes, reinterpret_cast<const unsigned char*>(key.data()),
                               static_cast<unsigned int>(key.size() * 8))
        != 0) {
        mbedtls_aes_free(&ctx->aes);
        delete ctx;
        return;
    }

    std::memcpy(ctx->nonce_counter, iv.data(), 16);
    std::memset(ctx->stream_block, 0, 16);
    ctx->nc_off = 0;

    ctx_ = ctx;
}

AesCtrCipher::~AesCtrCipher() {
    if (ctx_) {
        auto* ctx = static_cast<AesCtrContext*>(ctx_);
        mbedtls_aes_free(&ctx->aes);
        mbedtls_platform_zeroize(ctx->nonce_counter, 16);
        mbedtls_platform_zeroize(ctx->stream_block, 16);
        delete ctx;
    }
}

size_t AesCtrCipher::process(const uint8_t* in, uint8_t* out, size_t len) {
    if (!ctx_) {
        return 0;
    }
    auto* ctx = static_cast<AesCtrContext*>(ctx_);
    if (mbedtls_aes_crypt_ctr(&ctx->aes, len, &ctx->nc_off, ctx->nonce_counter, ctx->stream_block, in, out) != 0) {
        return 0;
    }
    return len;
}

bool RandomBytes(unsigned char* buf, size_t len) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctrDrbg);

    if (mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy, nullptr, 0) != 0
        || mbedtls_ctr_drbg_random(&ctrDrbg, buf, len) != 0) {
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        return false;
    }

    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return true;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
