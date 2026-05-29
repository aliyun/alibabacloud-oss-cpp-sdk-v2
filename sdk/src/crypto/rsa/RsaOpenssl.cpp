#include "../RsaUtils.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

class OpensslRsaPublicKey : public RsaPublicKey {
  public:
    explicit OpensslRsaPublicKey(EVP_PKEY* pkey) : pkey_(pkey) {}

    ~OpensslRsaPublicKey() override {
        if (pkey_) {
            EVP_PKEY_free(pkey_);
        }
    }

    OpensslRsaPublicKey(const OpensslRsaPublicKey&) = delete;
    OpensslRsaPublicKey& operator=(const OpensslRsaPublicKey&) = delete;

    std::string encrypt(const std::string& plaintext) override {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey_, nullptr);
        if (!ctx) {
            return {};
        }

        if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }

        size_t outLen = 0;
        if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char*>(plaintext.data()),
                             plaintext.size())
            <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }

        std::string result(outLen, '\0');
        if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outLen,
                             reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size())
            <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }
        result.resize(outLen);

        EVP_PKEY_CTX_free(ctx);
        return result;
    }

  private:
    EVP_PKEY* pkey_;
};

class OpensslRsaPrivateKey : public RsaPrivateKey {
  public:
    explicit OpensslRsaPrivateKey(EVP_PKEY* pkey) : pkey_(pkey) {}

    ~OpensslRsaPrivateKey() override {
        if (pkey_) {
            EVP_PKEY_free(pkey_);
        }
    }

    OpensslRsaPrivateKey(const OpensslRsaPrivateKey&) = delete;
    OpensslRsaPrivateKey& operator=(const OpensslRsaPrivateKey&) = delete;

    std::string decrypt(const std::string& ciphertext) override {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey_, nullptr);
        if (!ctx) {
            return {};
        }

        if (EVP_PKEY_decrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }

        size_t outLen = 0;
        if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char*>(ciphertext.data()),
                             ciphertext.size())
            <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }

        std::string result(outLen, '\0');
        if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outLen,
                             reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size())
            <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return {};
        }
        result.resize(outLen);

        EVP_PKEY_CTX_free(ctx);
        return result;
    }

  private:
    EVP_PKEY* pkey_;
};

} // namespace

std::unique_ptr<RsaPublicKey> tryRsaPublicKey(const std::string& publicKeyPem, std::string& detailError) {
    BIO* bio = BIO_new_mem_buf(publicKeyPem.data(), static_cast<int>(publicKeyPem.size()));
    if (!bio) {
        detailError = "BIO_new_mem_buf failed";
        return nullptr;
    }

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        detailError = "PEM_read_bio_PUBKEY failed";
        return nullptr;
    }

    return std::make_unique<OpensslRsaPublicKey>(pkey);
}

std::unique_ptr<RsaPrivateKey> tryRsaPrivateKey(const std::string& privateKeyPem, std::string& detailError) {
    BIO* bio = BIO_new_mem_buf(privateKeyPem.data(), static_cast<int>(privateKeyPem.size()));
    if (!bio) {
        detailError = "BIO_new_mem_buf failed";
        return nullptr;
    }

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        detailError = "PEM_read_bio_PrivateKey failed";
        return nullptr;
    }

    return std::make_unique<OpensslRsaPrivateKey>(pkey);
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
