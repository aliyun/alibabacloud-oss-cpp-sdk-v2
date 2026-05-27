#pragma once

#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

std::string RsaPublicEncrypt(const std::string& publicKeyPem, const std::string& plaintext);
std::string RsaPrivateDecrypt(const std::string& privateKeyPem, const std::string& ciphertext);

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
