#pragma once

#include "alibabacloud/oss2/crypto/MasterCipher.h"
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

/// Creates a MasterCipher that wraps/unwraps data keys with RSA PKCS#1 v1.5.
/// PEM keys are parsed once at construction and reused across threads.
/// Either key may be empty: pass empty publicKeyPem for decrypt-only,
/// or empty privateKeyPem for encrypt-only.
ALIBABACLOUD_OSS_API std::shared_ptr<MasterCipher> makeRsaMasterCipher(
    const std::string& publicKeyPem, const std::string& privateKeyPem,
    const std::map<std::string, std::string>& description = {});

/// Returns nullopt if the PEM is a valid RSA public key, or an error message otherwise.
ALIBABACLOUD_OSS_API std::optional<std::string> validateRsaPublicKey(const std::string& publicKeyPem);

/// Returns nullopt if the PEM is a valid RSA private key, or an error message otherwise.
ALIBABACLOUD_OSS_API std::optional<std::string> validateRsaPrivateKey(const std::string& privateKeyPem);

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
