#include "../RsaUtils.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/version.h>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

std::string RsaPublicEncrypt(const std::string& publicKeyPem, const std::string& plaintext) {
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
        return {};
    }

    if (mbedtls_pk_parse_public_key(&pk,
            reinterpret_cast<const unsigned char*>(publicKeyPem.data()),
            publicKeyPem.size() + 1) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        return {};
    }

    size_t outLen = mbedtls_pk_get_len(&pk);
    std::string result(outLen, '\0');

    if (mbedtls_pk_encrypt(&pk,
            reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size(),
            reinterpret_cast<unsigned char*>(result.data()), &outLen, result.size(),
            mbedtls_ctr_drbg_random, &ctrDrbg) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        return {};
    }
    result.resize(outLen);

    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return result;
}

std::string RsaPrivateDecrypt(const std::string& privateKeyPem, const std::string& ciphertext) {
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
        return {};
    }

    if (mbedtls_pk_parse_key(&pk,
            reinterpret_cast<const unsigned char*>(privateKeyPem.data()),
            privateKeyPem.size() + 1, nullptr, 0
#if MBEDTLS_VERSION_NUMBER >= 0x03000000
            , mbedtls_ctr_drbg_random, &ctrDrbg
#endif
            ) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        return {};
    }

    size_t outLen = mbedtls_pk_get_len(&pk);
    std::string result(outLen, '\0');

    if (mbedtls_pk_decrypt(&pk,
            reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size(),
            reinterpret_cast<unsigned char*>(result.data()), &outLen, result.size(),
            mbedtls_ctr_drbg_random, &ctrDrbg) != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctrDrbg);
        mbedtls_entropy_free(&entropy);
        return {};
    }
    result.resize(outLen);

    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return result;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
