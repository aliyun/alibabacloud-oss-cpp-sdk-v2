#pragma once

#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/crypto/ContentCipher.h"

namespace alibabacloud {
namespace oss2 {
namespace crypto {

class MasterCipher;

// Encryption-specific configuration, orthogonal to ClientConfiguration.
struct ALIBABACLOUD_OSS_API EncryptionConfiguration {
    // Master cipher for key wrapping (e.g. RsaMasterCipher).
    std::shared_ptr<MasterCipher> masterCipher;
};

// Holds the shared ContentCipher and partSize/dataSize for an in-progress
// encrypted multipart upload. Passed between initiateMultipartUpload and uploadPart.
class ALIBABACLOUD_OSS_API EncryptionMultiPartContext {
  public:
    static std::shared_ptr<EncryptionMultiPartContext> create(int64_t partSize, int64_t dataSize,
                                                              std::unique_ptr<ContentCipher> contentCipher) {
        return std::shared_ptr<EncryptionMultiPartContext>(
                new EncryptionMultiPartContext(partSize, dataSize, std::move(contentCipher)));
    }

    int64_t getPartSize() const {
        return partSize_;
    }
    int64_t getDataSize() const {
        return dataSize_;
    }
    const ContentCipher& getContentCipher() const {
        return *contentCipher_;
    }

  private:
    EncryptionMultiPartContext(int64_t partSize, int64_t dataSize, std::unique_ptr<ContentCipher> cc)
            : partSize_(partSize), dataSize_(dataSize), contentCipher_(std::move(cc)) {}

    int64_t partSize_;
    int64_t dataSize_;
    std::unique_ptr<ContentCipher> contentCipher_;
};

} // namespace crypto

// OSSClient wrapper that transparently encrypts uploads and decrypts downloads.
// Supports putObject, getObject (including range), and multipart upload.
class ALIBABACLOUD_OSS_API OSSEncryptionClient {
  public:
    OSSEncryptionClient(const ClientConfiguration& config, crypto::EncryptionConfiguration encryptionConfig);

    OSSEncryptionClient(const ClientConfiguration& config, ClientOptionsFns& fns,
                        crypto::EncryptionConfiguration encryptionConfig);

    ~OSSEncryptionClient();
    OSSEncryptionClient(OSSEncryptionClient&&) noexcept;
    OSSEncryptionClient& operator=(OSSEncryptionClient&&) noexcept;

    OSSClient& unwrap();
    const OSSClient& unwrap() const;

    PutObjectOutcome putObject(const models::PutObjectRequest& request, const OperationOptions* options = nullptr);

    GetObjectOutcome getObject(const models::GetObjectRequest& request, const OperationOptions* options = nullptr);

    InitiateMultipartUploadOutcome initiateMultipartUpload(const models::InitiateMultipartUploadRequest& request,
                                                           const OperationOptions* options = nullptr);

    UploadPartOutcome uploadPart(const models::UploadPartRequest& request, const OperationOptions* options = nullptr);

    CompleteMultipartUploadOutcome completeMultipartUpload(const models::CompleteMultipartUploadRequest& request,
                                                           const OperationOptions* options = nullptr);

    AbortMultipartUploadOutcome abortMultipartUpload(const models::AbortMultipartUploadRequest& request,
                                                     const OperationOptions* options = nullptr);

    ListPartsOutcome listParts(const models::ListPartsRequest& request, const OperationOptions* options = nullptr);

  private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
};

} // namespace oss2
} // namespace alibabacloud
