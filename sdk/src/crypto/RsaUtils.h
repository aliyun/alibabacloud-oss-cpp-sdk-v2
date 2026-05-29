#pragma once

#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

class RsaPublicKey {
  public:
    virtual ~RsaPublicKey() = default;
    virtual std::string encrypt(const std::string& plaintext) = 0;
};

class RsaPrivateKey {
  public:
    virtual ~RsaPrivateKey() = default;
    virtual std::string decrypt(const std::string& ciphertext) = 0;
};

std::unique_ptr<RsaPublicKey> tryRsaPublicKey(const std::string& publicKeyPem, std::string& detailError);

std::unique_ptr<RsaPrivateKey> tryRsaPrivateKey(const std::string& privateKeyPem, std::string& detailError);

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
