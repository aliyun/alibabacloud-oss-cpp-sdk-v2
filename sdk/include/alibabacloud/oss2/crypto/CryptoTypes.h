#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

// Per-object key material used for content encryption/decryption.
struct ALIBABACLOUD_OSS_API CipherData {
    std::string key;          // plaintext data encryption key (DEK)
    std::string iv;           // plaintext initialization vector
    std::string encryptedKey; // DEK encrypted by the master cipher
    std::string encryptedIV;  // IV encrypted by the master cipher
};

// Algorithm descriptors associated with the ContentCipherBuilder.
struct ALIBABACLOUD_OSS_API CipherMetadata {
    std::string wrapAlgorithm; // key-wrap algorithm, e.g. "RSA/NONE/PKCS1Padding"
    std::string cekAlgorithm;  // content encryption algorithm, e.g. "AES/CTR/NoPadding"
    std::string matDesc;       // JSON material description for key rotation
};

// Crypto envelope extracted from object metadata headers on download.
struct ALIBABACLOUD_OSS_API Envelope {
    std::string iv;
    std::string cipherKey;
    std::string matDesc;
    std::string wrapAlg;
    std::string cekAlg;
    std::string unencryptedMD5;
    std::string unencryptedContentLength;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
