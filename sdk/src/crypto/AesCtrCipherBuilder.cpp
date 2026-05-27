#include "AesCtrCipherBuilder.h"
#include "AesCtrContentCipher.h"
#include "Aes256Utils.h"
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

std::unique_ptr<ContentCipher> AesCtrCipherBuilder::create() {
    CipherData cd;
    if (!randomKeyIV(cd, 32, 16)) return nullptr;
    cd.encryptedKey = masterCipher_->encrypt(cd.key);
    cd.encryptedIV = masterCipher_->encrypt(cd.iv);
    if (cd.encryptedKey.empty() || cd.encryptedIV.empty()) return nullptr;
    return std::make_unique<AesCtrContentCipher>(std::move(cd));
}

std::unique_ptr<ContentCipher> AesCtrCipherBuilder::fromEnvelope(const Envelope& envelope) {
    CipherData cd;
    cd.key = masterCipher_->decrypt(utils::Base64Decode(envelope.cipherKey));
    cd.iv = masterCipher_->decrypt(utils::Base64Decode(envelope.iv));
    if (cd.key.empty() || cd.iv.empty()) return nullptr;
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
