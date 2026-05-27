#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

// Abstract interface for the master key that wraps/unwraps data encryption keys.
// Implementations hold the actual key material (e.g. RSA key pair).
class ALIBABACLOUD_OSS_API MasterCipher {
  public:
    virtual ~MasterCipher() = default;
    virtual std::string encrypt(const std::string& plaintext) const = 0;
    virtual std::string decrypt(const std::string& ciphertext) const = 0;
    virtual std::string getWrapAlgorithm() const = 0;
    virtual std::string getMatDesc() const = 0;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
