#include "../Aes256Utils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

AesCtrCipher::AesCtrCipher(const std::string& key, const std::string& iv) : ctx_(nullptr) {
    EVP_CIPHER_CTX* evpCtx = EVP_CIPHER_CTX_new();
    if (!evpCtx) return;

    const EVP_CIPHER* cipher = nullptr;
    if (key.size() == 32) {
        cipher = EVP_aes_256_ctr();
    } else if (key.size() == 16) {
        cipher = EVP_aes_128_ctr();
    } else {
        EVP_CIPHER_CTX_free(evpCtx);
        return;
    }

    if (EVP_EncryptInit_ex(evpCtx, cipher, nullptr,
                           reinterpret_cast<const unsigned char*>(key.data()),
                           reinterpret_cast<const unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(evpCtx);
        return;
    }

    ctx_ = evpCtx;
}

AesCtrCipher::~AesCtrCipher() {
    if (ctx_) {
        EVP_CIPHER_CTX_free(static_cast<EVP_CIPHER_CTX*>(ctx_));
    }
}

size_t AesCtrCipher::process(const uint8_t* in, uint8_t* out, size_t len) {
    if (!ctx_) return 0;
    int outLen = 0;
    EVP_EncryptUpdate(static_cast<EVP_CIPHER_CTX*>(ctx_), out, &outLen, in, static_cast<int>(len));
    return static_cast<size_t>(outLen);
}

bool RandomBytes(unsigned char* buf, size_t len) {
    return RAND_bytes(buf, static_cast<int>(len)) == 1;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
