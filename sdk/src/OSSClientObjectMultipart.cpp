
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeObjectMultipart.h"
#include "src/utils/Utils.h"

namespace alibabacloud {
namespace oss2 {

#define requiredFiled(field)                                                                             \
    do {                                                                                                 \
        if (request.get##field().empty()) {                                                              \
            return OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                       \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field ""}}); \
        }                                                                                                \
    } while (false)


#define requiredFileds(field1, field2)                                                                     \
    do {                                                                                                   \
        if (request.get##field1().empty() && request.get##field2().empty()) {                              \
            return OperationError(                                                                         \
                    SdkErrorCode::ARGUMENT_REQUIRED,                                                       \
                    {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field1 " or " #field2 ""}}); \
        }                                                                                                  \
    } while (false)


#define requiredIntFiled(field)                                                                          \
    do {                                                                                                 \
        if (request.get##field() < 0) {                                                                  \
            return OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                       \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field ""}}); \
        }                                                                                                \
    } while (false)


#define requiredHasFiled(field)                                                                          \
    do {                                                                                                 \
        if (!request.has##field()) {                                                                     \
            return OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                       \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field ""}}); \
        }                                                                                                \
    } while (false)


InitiateMultipartUploadOutcome OSSClient::initiateMultipartUpload(const models::InitiateMultipartUploadRequest& request,
                                                                  const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);

    auto input = transform::fromInitiateMultipartUpload(request);
    if (input.headers.find("Content-Type") == input.headers.end()) {
        input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
    }
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toInitiateMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

UploadPartOutcome OSSClient::uploadPart(const models::UploadPartRequest& request, const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);
    requiredIntFiled(PartNumber);
    requiredFiled(UploadId);

    auto input = transform::fromUploadPart(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toUploadPart(std::move(std::get<OperationOutput>(result)));
}

CompleteMultipartUploadOutcome OSSClient::completeMultipartUpload(const models::CompleteMultipartUploadRequest& request,
                                                                  const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);
    requiredFiled(UploadId);

    auto input = transform::fromCompleteMultipartUpload(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toCompleteMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

UploadPartCopyOutcome OSSClient::uploadPartCopy(const models::UploadPartCopyRequest& request,
                                                const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);
    requiredIntFiled(PartNumber);
    requiredFiled(UploadId);
    requiredFileds(SourceKey, CopySource);

    auto input = transform::fromUploadPartCopy(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toUploadPartCopy(std::move(std::get<OperationOutput>(result)));
}

AbortMultipartUploadOutcome OSSClient::abortMultipartUpload(const models::AbortMultipartUploadRequest& request,
                                                            const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);
    requiredFiled(UploadId);

    auto input = transform::fromAbortMultipartUpload(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toAbortMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

ListMultipartUploadsOutcome OSSClient::listMultipartUploads(const models::ListMultipartUploadsRequest& request,
                                                            const OperationOptions* options) {
    requiredFiled(Bucket);

    auto input = transform::fromListMultipartUploads(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toListMultipartUploads(std::move(std::get<OperationOutput>(result)));
}

ListPartsOutcome OSSClient::listParts(const models::ListPartsRequest& request, const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);
    requiredFiled(UploadId);

    auto input = transform::fromListParts(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toListParts(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
