#include "alibabacloud/oss2/crypto/Error.h"
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

class crypto_error_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.crypto"; }

    std::string message(int ev) const override {
        switch (static_cast<CryptoErrorCode>(ev)) {
            case CryptoErrorCode::EncryptFailed:
                return "encryption failed";
            case CryptoErrorCode::DecryptFailed:
                return "decryption failed";
            case CryptoErrorCode::InvalidKey:
                return "invalid key";
            case CryptoErrorCode::RandomGenerationFailed:
                return "random key/iv generation failed";
            case CryptoErrorCode::EmptyKeyOrIV:
                return "decrypted key or iv is empty";
            case CryptoErrorCode::InvalidKeyOrIV:
                return "decrypted key or iv has invalid length";
            default:
                return "unknown crypto error";
        }
    }

    bool equivalent(int, const std::error_condition& cond) const noexcept override {
        return cond == make_error_condition(ErrorCondition::NonRetryable);
    }
};

} // namespace

std::error_code make_error_code(CryptoErrorCode e) {
    static const crypto_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
