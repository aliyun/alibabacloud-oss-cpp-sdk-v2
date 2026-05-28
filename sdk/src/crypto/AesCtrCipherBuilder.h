#pragma once

#include "alibabacloud/oss2/crypto/ContentCipher.h"
#include "alibabacloud/oss2/crypto/MasterCipher.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

std::unique_ptr<ContentCipherBuilder> CreateAesCtrCipherBuilder(
    std::shared_ptr<MasterCipher> masterCipher);

class AesCtrCipherBuilder : public ContentCipherBuilder {
  public:
    explicit AesCtrCipherBuilder(std::shared_ptr<MasterCipher> masterCipher);

    ContentCipherResult create() override;
    ContentCipherResult fromEnvelope(const Envelope& envelope) override;
    const CipherMetadata& getCipherMetadata() const override;
    int getAlignLen() const override;

  private:
    std::shared_ptr<MasterCipher> masterCipher_;
    CipherMetadata metadata_;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
