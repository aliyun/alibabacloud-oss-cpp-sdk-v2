#include "../RsaUtils.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

std::string RsaPublicEncrypt(const std::string& publicKeyPem, const std::string& plaintext) {
    BIO* bio = BIO_new_mem_buf(publicKeyPem.data(), static_cast<int>(publicKeyPem.size()));
    if (!bio) return {};

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) return {};

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return {};
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    std::string result(outLen, '\0');
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outLen,
                         reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    result.resize(outLen);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return result;
}

std::string RsaPrivateDecrypt(const std::string& privateKeyPem, const std::string& ciphertext) {
    BIO* bio = BIO_new_mem_buf(privateKeyPem.data(), static_cast<int>(privateKeyPem.size()));
    if (!bio) return {};

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) return {};

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return {};
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    std::string result(outLen, '\0');
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outLen,
                         reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    result.resize(outLen);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return result;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
