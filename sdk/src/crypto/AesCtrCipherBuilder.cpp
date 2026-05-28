#include "AesCtrCipherBuilder.h"
#include "AesCtrContentCipher.h"
#include "Aes256Utils.h"
#include "alibabacloud/oss2/crypto/Error.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

constexpr const char* kAesCtrAlgorithm = "AES/CTR/NoPadding";

bool randomKeyIV(CipherData& cd, int keyLen, int ivLen) {
    cd.key.resize(static_cast<size_t>(keyLen));
    cd.iv.resize(static_cast<size_t>(ivLen));
    return RandomBytes(reinterpret_cast<unsigned char*>(cd.key.data()), cd.key.size()) &&
           RandomBytes(reinterpret_cast<unsigned char*>(cd.iv.data()), cd.iv.size());
}

} // namespace

AesCtrCipherBuilder::AesCtrCipherBuilder(std::shared_ptr<MasterCipher> masterCipher)
    : masterCipher_(masterCipher)
    , metadata_{masterCipher->getWrapAlgorithm(), kAesCtrAlgorithm, masterCipher->getMatDesc()} {}

ContentCipherResult AesCtrCipherBuilder::create() {
    CipherData cd;
    if (!randomKeyIV(cd, 32, 16)) {
        return CryptoErrorCode::RandomGenerationFailed;
    }
    auto keyResult = masterCipher_->encrypt(cd.key);
    if (auto* ec = std::get_if<std::error_code>(&keyResult)) {
        return *ec;
    }
    auto ivResult = masterCipher_->encrypt(cd.iv);
    if (auto* ec = std::get_if<std::error_code>(&ivResult)) {
        return *ec;
    }
    cd.encryptedKey = std::move(std::get<std::string>(keyResult));
    cd.encryptedIV = std::move(std::get<std::string>(ivResult));
    return std::make_unique<AesCtrContentCipher>(std::move(cd));
}

ContentCipherResult AesCtrCipherBuilder::fromEnvelope(const Envelope& envelope) {
    if (envelope.cipherKey.empty() || envelope.iv.empty()) {
        return CryptoErrorCode::EmptyKeyOrIV;
    }

    auto keyResult = masterCipher_->decrypt(utils::Base64Decode(envelope.cipherKey));
    if (auto* ec = std::get_if<std::error_code>(&keyResult)) {
        return *ec;
    }
    auto ivResult = masterCipher_->decrypt(utils::Base64Decode(envelope.iv));
    if (auto* ec = std::get_if<std::error_code>(&ivResult)) {
        return *ec;
    }

    CipherData cd;
    cd.key = std::move(std::get<std::string>(keyResult));
    cd.iv = std::move(std::get<std::string>(ivResult));
    if (cd.key.size() != 32 || cd.iv.size() != 16) {
        return CryptoErrorCode::InvalidKeyOrIV;
    }
    cd.encryptedKey = envelope.cipherKey;
    cd.encryptedIV = envelope.iv;
    return std::make_unique<AesCtrContentCipher>(std::move(cd));
}

const CipherMetadata& AesCtrCipherBuilder::getCipherMetadata() const {
    return metadata_;
}

int AesCtrCipherBuilder::getAlignLen() const {
    return 16;
}

std::unique_ptr<ContentCipherBuilder> CreateAesCtrCipherBuilder(
    std::shared_ptr<MasterCipher> masterCipher) {
    return std::make_unique<AesCtrCipherBuilder>(std::move(masterCipher));
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
