#include "../RsaUtils.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/version.h>

#include <mutex>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

class MbedtlsRsaPublicKey : public RsaPublicKey {
  public:
    // Ownership transfer via value copy: caller must not free the originals.
    MbedtlsRsaPublicKey(mbedtls_pk_context pk, mbedtls_entropy_context entropy, mbedtls_ctr_drbg_context ctrDrbg)
        : pk_(pk), entropy_(entropy), ctrDrbg_(ctrDrbg) {}

    ~MbedtlsRsaPublicKey() override {
        mbedtls_pk_free(&pk_);
        mbedtls_ctr_drbg_free(&ctrDrbg_);
        mbedtls_entropy_free(&entropy_);
    }

    MbedtlsRsaPublicKey(const MbedtlsRsaPublicKey&) = delete;
    MbedtlsRsaPublicKey& operator=(const MbedtlsRsaPublicKey&) = delete;

    std::string encrypt(const std::string& plaintext) override {
        std::lock_guard<std::mutex> lock(mutex_);

        size_t outLen = mbedtls_pk_get_len(&pk_);
        std::string result(outLen, '\0');

        if (mbedtls_pk_encrypt(&pk_, reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size(),
                               reinterpret_cast<unsigned char*>(result.data()), &outLen, result.size(),
                               mbedtls_ctr_drbg_random, &ctrDrbg_)
            != 0) {
            return {};
        }
        result.resize(outLen);
        return result;
    }

  private:
    mbedtls_pk_context pk_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctrDrbg_;
    std::mutex mutex_;
};

class MbedtlsRsaPrivateKey : public RsaPrivateKey {
  public:
    // Ownership transfer via value copy: caller must not free the originals.
    MbedtlsRsaPrivateKey(mbedtls_pk_context pk, mbedtls_entropy_context entropy, mbedtls_ctr_drbg_context ctrDrbg)
        : pk_(pk), entropy_(entropy), ctrDrbg_(ctrDrbg) {}

    ~MbedtlsRsaPrivateKey() override {
        mbedtls_pk_free(&pk_);
        mbedtls_ctr_drbg_free(&ctrDrbg_);
        mbedtls_entropy_free(&entropy_);
    }

    MbedtlsRsaPrivateKey(const MbedtlsRsaPrivateKey&) = delete;
    MbedtlsRsaPrivateKey& operator=(const MbedtlsRsaPrivateKey&) = delete;

    std::string decrypt(const std::string& ciphertext) override {
        std::lock_guard<std::mutex> lock(mutex_);

        size_t outLen = mbedtls_pk_get_len(&pk_);
        std::string result(outLen, '\0');

        if (mbedtls_pk_decrypt(&pk_, reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size(),
                               reinterpret_cast<unsigned char*>(result.data()), &outLen, result.size(),
                               mbedtls_ctr_drbg_random, &ctrDrbg_)
            != 0) {
            return {};
        }
        result.resize(outLen);
        return result;
    }

  private:
    mbedtls_pk_context pk_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctrDrbg_;
    std::mutex mutex_;
};

} // namespace

std::unique_ptr<RsaPublicKey> tryRsaPublicKey(const std::string& publicKeyPem, std::string& detailError) {
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctrDrbg);

    if (mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy, nullptr, 0) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        detailError = "mbedtls_ctr_drbg_seed failed";
        return nullptr;
    }

    int ret = mbedtls_pk_parse_public_key(&pk, reinterpret_cast<const unsigned char*>(publicKeyPem.data()),
                                          publicKeyPem.size() + 1);
    if (ret != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        detailError = "mbedtls_pk_parse_public_key returned " + std::to_string(ret);
        return nullptr;
    }

    return std::make_unique<MbedtlsRsaPublicKey>(pk, entropy, ctrDrbg);
}

std::unique_ptr<RsaPrivateKey> tryRsaPrivateKey(const std::string& privateKeyPem, std::string& detailError) {
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctrDrbg);

    if (mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy, nullptr, 0) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        detailError = "mbedtls_ctr_drbg_seed failed";
        return nullptr;
    }

    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char*>(privateKeyPem.data()),
                                   privateKeyPem.size() + 1, nullptr, 0
#if MBEDTLS_VERSION_NUMBER >= 0x03000000
                                   ,
                                   mbedtls_ctr_drbg_random, &ctrDrbg
#endif
    );
    if (ret != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        detailError = "mbedtls_pk_parse_key returned " + std::to_string(ret);
        return nullptr;
    }

    return std::make_unique<MbedtlsRsaPrivateKey>(pk, entropy, ctrDrbg);
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
