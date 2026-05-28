#pragma once

#include "alibabacloud/oss2/crypto/MasterCipher.h"
#include <map>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

// MasterCipher implementation using RSA PKCS#1 v1.5 for key wrapping.
// The description map is serialized as JSON and stored in the matdesc header
// to support key rotation (identify which RSA key pair was used).
class ALIBABACLOUD_OSS_API MasterRsaCipher : public MasterCipher {
  public:
    MasterRsaCipher(const std::string& publicKeyPem, const std::string& privateKeyPem,
                    const std::map<std::string, std::string>& description = {});
    ~MasterRsaCipher() override;

    MasterCipherResult encrypt(const std::string& plaintext) const override;
    MasterCipherResult decrypt(const std::string& ciphertext) const override;
    std::string getWrapAlgorithm() const override;
    std::string getMatDesc() const override;

  private:
    std::string publicKeyPem_;
    std::string privateKeyPem_;
    std::string matDesc_;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
