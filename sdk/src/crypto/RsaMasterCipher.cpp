#include "alibabacloud/oss2/crypto/RsaMasterCipher.h"
#include "RsaUtils.h"
#include "alibabacloud/oss2/crypto/Error.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

constexpr const char* kRsaCryptoWrap = "RSA/NONE/PKCS1Padding";

std::string jsonEscape(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        if (c == '"') {
            result += "\\\"";
        } else if (c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    return result;
}

std::string serializeMatDesc(const std::map<std::string, std::string>& desc) {
    if (desc.empty()) {
        return "{}";
    }
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& kv : desc) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << jsonEscape(kv.first) << "\":\"" << jsonEscape(kv.second) << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

class RsaMasterCipher : public MasterCipher {
  public:
    RsaMasterCipher(const std::string& publicKeyPem, const std::string& privateKeyPem,
                    const std::map<std::string, std::string>& description)
        : matDesc_(serializeMatDesc(description)) {
        std::string error;
        if (!publicKeyPem.empty()) {
            publicKey_ = tryRsaPublicKey(publicKeyPem, error);
        }
        if (!privateKeyPem.empty()) {
            privateKey_ = tryRsaPrivateKey(privateKeyPem, error);
        }
    }

    MasterCipherResult encrypt(const std::string& plaintext) const override {
        if (!publicKey_) {
            return CryptoErrorCode::InvalidKey;
        }
        auto result = publicKey_->encrypt(plaintext);
        if (result.empty()) {
            return CryptoErrorCode::EncryptFailed;
        }
        return result;
    }

    MasterCipherResult decrypt(const std::string& ciphertext) const override {
        if (!privateKey_) {
            return CryptoErrorCode::InvalidKey;
        }
        auto result = privateKey_->decrypt(ciphertext);
        if (result.empty()) {
            return CryptoErrorCode::DecryptFailed;
        }
        return result;
    }

    std::string getWrapAlgorithm() const override {
        return kRsaCryptoWrap;
    }

    std::string getMatDesc() const override {
        return matDesc_;
    }

  private:
    std::shared_ptr<RsaPublicKey> publicKey_;
    std::shared_ptr<RsaPrivateKey> privateKey_;
    std::string matDesc_;
};

} // namespace

std::shared_ptr<MasterCipher> makeRsaMasterCipher(const std::string& publicKeyPem, const std::string& privateKeyPem,
                                                  const std::map<std::string, std::string>& description) {
    return std::make_shared<RsaMasterCipher>(publicKeyPem, privateKeyPem, description);
}

std::optional<std::string> validateRsaPublicKey(const std::string& publicKeyPem) {
    std::string detailError;
    auto key = tryRsaPublicKey(publicKeyPem, detailError);
    if (!key) {
        return detailError;
    }
    return std::nullopt;
}

std::optional<std::string> validateRsaPrivateKey(const std::string& privateKeyPem) {
    std::string detailError;
    auto key = tryRsaPrivateKey(privateKeyPem, detailError);
    if (!key) {
        return detailError;
    }
    return std::nullopt;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
