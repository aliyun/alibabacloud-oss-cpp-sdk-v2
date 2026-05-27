#include "../Aes256Utils.h"

#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

AesCtrCipher::AesCtrCipher(const std::string& key, const std::string& iv) : ctx_(nullptr) {
    auto* cipherCtx = new (std::nothrow) mbedtls_cipher_context_t;
    if (!cipherCtx) return;
    mbedtls_cipher_init(cipherCtx);

    const mbedtls_cipher_info_t* cipherInfo = nullptr;
    if (key.size() == 32) {
        cipherInfo = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CTR);
    } else if (key.size() == 16) {
        cipherInfo = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
    } else {
        mbedtls_cipher_free(cipherCtx);
        delete cipherCtx;
        return;
    }

    if (mbedtls_cipher_setup(cipherCtx, cipherInfo) != 0 ||
        mbedtls_cipher_setkey(cipherCtx, reinterpret_cast<const unsigned char*>(key.data()),
                              static_cast<int>(key.size() * 8), MBEDTLS_ENCRYPT) != 0 ||
        mbedtls_cipher_set_iv(cipherCtx, reinterpret_cast<const unsigned char*>(iv.data()),
                              iv.size()) != 0 ||
        mbedtls_cipher_reset(cipherCtx) != 0) {
        mbedtls_cipher_free(cipherCtx);
        delete cipherCtx;
        return;
    }

    ctx_ = cipherCtx;
}

AesCtrCipher::~AesCtrCipher() {
    if (ctx_) {
        auto* cipherCtx = static_cast<mbedtls_cipher_context_t*>(ctx_);
        mbedtls_cipher_free(cipherCtx);
        delete cipherCtx;
    }
}

size_t AesCtrCipher::process(const uint8_t* in, uint8_t* out, size_t len) {
    if (!ctx_) return 0;
    size_t outLen = 0;
    mbedtls_cipher_update(static_cast<mbedtls_cipher_context_t*>(ctx_), in, len, out, &outLen);
    return outLen;
}

bool RandomBytes(unsigned char* buf, size_t len) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctrDrbg);

    if (mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy, nullptr, 0) != 0 ||
        mbedtls_ctr_drbg_random(&ctrDrbg, buf, len) != 0) {
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
