#include "alibabacloud/oss2/crypto/MasterRsaCipher.h"
#include "RsaUtils.h"

#include <cstring>
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
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else result += c;
    }
    return result;
}

std::string serializeMatDesc(const std::map<std::string, std::string>& desc) {
    if (desc.empty()) return "{}";
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& kv : desc) {
        if (!first) oss << ",";
        oss << "\"" << jsonEscape(kv.first) << "\":\"" << jsonEscape(kv.second) << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

} // namespace

namespace {

void secureZero(std::string& s) {
    if (!s.empty()) {
        volatile char* p = &s[0];
        std::size_t n = s.size();
        while (n--) *p++ = 0;
    }
}

} // namespace

MasterRsaCipher::MasterRsaCipher(const std::string& publicKeyPem, const std::string& privateKeyPem,
                                 const std::map<std::string, std::string>& description)
        : publicKeyPem_(publicKeyPem), privateKeyPem_(privateKeyPem), matDesc_(serializeMatDesc(description)) {}

MasterRsaCipher::~MasterRsaCipher() {
    secureZero(privateKeyPem_);
}

std::string MasterRsaCipher::getWrapAlgorithm() const {
    return kRsaCryptoWrap;
}

std::string MasterRsaCipher::getMatDesc() const {
    return matDesc_;
}

std::string MasterRsaCipher::encrypt(const std::string& plaintext) const {
    return RsaPublicEncrypt(publicKeyPem_, plaintext);
}

std::string MasterRsaCipher::decrypt(const std::string& ciphertext) const {
    return RsaPrivateDecrypt(privateKeyPem_, ciphertext);
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
