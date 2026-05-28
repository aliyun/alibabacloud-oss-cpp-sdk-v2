#pragma once

#include "alibabacloud/oss2/crypto/ContentCipher.h"

namespace alibabacloud {
namespace oss2 {
namespace crypto {

class AesCtrContentCipher : public ContentCipher {
  public:
    explicit AesCtrContentCipher(CipherData cd);

    std::shared_ptr<ByteContent> encryptContent(std::shared_ptr<ByteContent> body) override;

    std::shared_ptr<ByteWriter> decryptContent(
        std::shared_ptr<ByteWriter> writer,
        int64_t encryptedContentLen) override;

    int64_t getEncryptedLen(int64_t plainLen) const override;

    const CipherData& getCipherData() const override;

    std::unique_ptr<ContentCipher> clone() const override;

    void seekTo(uint64_t offset) override;

    int getAlignLen() const override;

  private:
    CipherData cipherData_;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
