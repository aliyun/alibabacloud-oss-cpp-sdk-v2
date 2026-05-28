#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <string>
#include <system_error>
#include <variant>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

using MasterCipherResult = std::variant<std::string, std::error_code>;

// Abstract interface for the master key that wraps/unwraps data encryption keys.
// Implementations hold the actual key material (e.g. RSA key pair).
class ALIBABACLOUD_OSS_API MasterCipher {
  public:
    virtual ~MasterCipher() = default;
    virtual MasterCipherResult encrypt(const std::string& plaintext) const = 0;
    virtual MasterCipherResult decrypt(const std::string& ciphertext) const = 0;
    virtual std::string getWrapAlgorithm() const = 0;
    virtual std::string getMatDesc() const = 0;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
