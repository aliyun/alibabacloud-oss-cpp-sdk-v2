#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include "alibabacloud/oss2/crypto/CryptoTypes.h"

#include <cstdint>
#include <memory>
#include <system_error>
#include <variant>

namespace alibabacloud {
namespace oss2 {

class ByteContent;
class ByteWriter;

namespace crypto {

// Performs streaming encryption and decryption for a single object.
// Each instance holds per-object key material (CipherData).
class ALIBABACLOUD_OSS_API ContentCipher {
  public:
    virtual ~ContentCipher() = default;

    // Wraps an upload body with encryption; the returned ByteContent
    // produces ciphertext as it is read.
    virtual std::shared_ptr<ByteContent> encryptContent(std::shared_ptr<ByteContent> body) = 0;

    // Wraps a download writer with decryption; plaintext is forwarded to
    // the inner writer as ciphertext arrives.
    virtual std::shared_ptr<ByteWriter> decryptContent(std::shared_ptr<ByteWriter> writer,
                                                       int64_t encryptedContentLen) = 0;

    virtual int64_t getEncryptedLen(int64_t plainLen) const = 0;

    virtual const CipherData& getCipherData() const = 0;

    // Deep copy; used to create independent per-part ciphers in multipart upload.
    virtual std::unique_ptr<ContentCipher> clone() const = 0;

    // Advance the cipher stream to the given byte offset (for range get / multipart).
    virtual void seekTo(uint64_t offset) = 0;

    // Block alignment length in bytes (e.g. 16 for AES).
    virtual int getAlignLen() const = 0;
};

using ContentCipherResult = std::variant<std::unique_ptr<ContentCipher>, std::error_code>;

// Factory that creates ContentCipher instances.
// On upload: create() generates fresh key material.
// On download: fromEnvelope() restores key material from stored headers.
class ALIBABACLOUD_OSS_API ContentCipherBuilder {
  public:
    virtual ~ContentCipherBuilder() = default;

    virtual ContentCipherResult create() = 0;

    virtual ContentCipherResult fromEnvelope(const Envelope& envelope) = 0;

    virtual const CipherMetadata& getCipherMetadata() const = 0;

    virtual int getAlignLen() const = 0;
};

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
