#include "AesCtrContentCipher.h"
#include "Aes256Utils.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

namespace {

class CryptoByteSource final : public ByteSource {
  public:
    CryptoByteSource(std::unique_ptr<ByteSource> inner, const std::string& key, const std::string& iv)
        : inner_(std::move(inner)), cipher_(std::make_unique<AesCtrCipher>(key, iv)) {}

  private:
    std::size_t onRead(std::uint8_t* buffer, std::size_t count) override {
        std::size_t bytesRead = inner_->read(buffer, count);
        if (bytesRead > 0 && cipher_->process(buffer, buffer, bytesRead) != bytesRead) {
            cipherError_ = true;
            return 0;
        }
        return bytesRead;
    }

    int iostate() override {
        if (cipherError_) return std::ios_base::badbit;
        return inner_->state();
    }

    std::unique_ptr<ByteSource> inner_;
    std::unique_ptr<AesCtrCipher> cipher_;
    bool cipherError_{false};
};

class CryptoByteContent final : public ByteContent {
  public:
    CryptoByteContent(std::shared_ptr<ByteContent> inner, const std::string& key, const std::string& iv)
        : inner_(std::move(inner)), key_(key), iv_(iv) {}

    std::optional<std::size_t> length() const override { return inner_->length(); }
    bool isOneShot() const override { return inner_->isOneShot(); }

    std::unique_ptr<ByteSource> spanSource() override {
        return std::make_unique<CryptoByteSource>(inner_->spanSource(), key_, iv_);
    }

  private:
    std::shared_ptr<ByteContent> inner_;
    std::string key_;
    std::string iv_;
};

class DecryptingWriter final : public ByteWriter {
  public:
    DecryptingWriter(std::shared_ptr<ByteWriter> inner, const std::string& key, const std::string& iv)
        : inner_(std::move(inner)), cipher_(std::make_unique<AesCtrCipher>(key, iv)) {}

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override {
        if (n == 0) return 0;
        if (buffer_.size() < n) buffer_.resize(n);
        if (cipher_->process(data, buffer_.data(), n) != n) {
            cipherError_ = true;
            return 0;
        }
        return inner_->write(buffer_.data(), n);
    }

    int iostate() const override {
        if (cipherError_) return std::ios_base::badbit;
        return inner_->state();
    }

    std::shared_ptr<ByteWriter> inner_;
    std::unique_ptr<AesCtrCipher> cipher_;
    std::vector<uint8_t> buffer_;
    bool cipherError_{false};
};

static void seekIV(std::string& iv, uint64_t offset) {
    if (iv.size() < 16) return;
    uint64_t blockIndex = offset / 16;

    uint64_t counter = 0;
    for (int i = 8; i < 16; i++) {
        counter = (counter << 8) | (static_cast<uint8_t>(iv[static_cast<size_t>(i)]));
    }
    counter += blockIndex;
    for (int i = 15; i >= 8; i--) {
        iv[static_cast<size_t>(i)] = static_cast<char>(counter & 0xFF);
        counter >>= 8;
    }
}

} // namespace

AesCtrContentCipher::AesCtrContentCipher(CipherData cd)
    : cipherData_(std::move(cd)) {}

std::shared_ptr<ByteContent> AesCtrContentCipher::encryptContent(std::shared_ptr<ByteContent> body) {
    return std::make_shared<CryptoByteContent>(std::move(body), cipherData_.key, cipherData_.iv);
}

std::shared_ptr<ByteWriter> AesCtrContentCipher::decryptContent(
    std::shared_ptr<ByteWriter> writer,
    int64_t /*encryptedContentLen*/) {
    return std::make_shared<DecryptingWriter>(std::move(writer), cipherData_.key, cipherData_.iv);
}

int64_t AesCtrContentCipher::getEncryptedLen(int64_t plainLen) const {
    return plainLen;
}

const CipherData& AesCtrContentCipher::getCipherData() const {
    return cipherData_;
}

std::unique_ptr<ContentCipher> AesCtrContentCipher::clone() const {
    return std::make_unique<AesCtrContentCipher>(cipherData_);
}

void AesCtrContentCipher::seekTo(uint64_t offset) {
    seekIV(cipherData_.iv, offset);
}

int AesCtrContentCipher::getAlignLen() const {
    return 16;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
