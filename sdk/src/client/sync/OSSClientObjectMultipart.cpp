
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ByteStreamUtils.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeObjectMultipart.h"
#include "src/utils/Utils.h"
#include "OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


InitiateMultipartUploadOutcome OSSClient::initiateMultipartUpload(const models::InitiateMultipartUploadRequest& request,
                                                                  const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromInitiateMultipartUpload(request);
    if (client_->hasFlag(FeatureFlagsType::AutoDetectMimeType)) {
        if (input.headers.find("Content-Type") == input.headers.end()) {
            // cppcheck-suppress stlFindInsert
            input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
        }
    }
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toInitiateMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

UploadPartOutcome OSSClient::uploadPart(const models::UploadPartRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(PartNumber);
    requiredField(UploadId);

    auto input = transform::fromUploadPart(request);

    internal::OperationInnerOptions innerOpts;
    if (request.getProgressCallback().has_value()) {
        int64_t total = 0;
        if (request.getBody()) {
            auto len = request.getBody()->length();
            total = len.has_value() ? static_cast<int64_t>(len.value()) : -1;
        }
        innerOpts.uploadObserver.push_back(
                std::make_shared<internal::ProgressObserver>(request.getProgressCallback().value(), total));
    }

    auto result = client_->Execute(input, options, &innerOpts);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toUploadPart(std::move(std::get<OperationOutput>(result)));
}

CompleteMultipartUploadOutcome OSSClient::completeMultipartUpload(const models::CompleteMultipartUploadRequest& request,
                                                                  const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(UploadId);

    auto input = transform::fromCompleteMultipartUpload(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toCompleteMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

UploadPartCopyOutcome OSSClient::uploadPartCopy(const models::UploadPartCopyRequest& request,
                                                const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(PartNumber);
    requiredField(UploadId);
    requiredFieldsOr(SourceKey, CopySource);

    auto input = transform::fromUploadPartCopy(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toUploadPartCopy(std::move(std::get<OperationOutput>(result)));
}

AbortMultipartUploadOutcome OSSClient::abortMultipartUpload(const models::AbortMultipartUploadRequest& request,
                                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(UploadId);

    auto input = transform::fromAbortMultipartUpload(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toAbortMultipartUpload(std::move(std::get<OperationOutput>(result)));
}

ListMultipartUploadsOutcome OSSClient::listMultipartUploads(const models::ListMultipartUploadsRequest& request,
                                                            const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromListMultipartUploads(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toListMultipartUploads(std::move(std::get<OperationOutput>(result)));
}

ListPartsOutcome OSSClient::listParts(const models::ListPartsRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(UploadId);

    auto input = transform::fromListParts(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toListParts(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
