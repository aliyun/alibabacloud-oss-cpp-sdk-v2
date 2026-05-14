
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeObjectMultipart.h"
#include "src/utils/Utils.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::initiateMultipartUploadAsync(const models::InitiateMultipartUploadRequest& request,
                                                    const InitiateMultipartUploadAsyncCallback& callback,
                                                    const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromInitiateMultipartUpload(request);
    if (input.headers.find("Content-Type") == input.headers.end()) {
        // cppcheck-suppress stlFindInsert
        input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
    }
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toInitiateMultipartUpload(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::uploadPartAsync(const models::UploadPartRequest& request,
                                      const UploadPartAsyncCallback& callback,
                                      const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(PartNumber);
    requiredFieldAsync(UploadId);

    auto input = transform::fromUploadPart(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toUploadPart(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::completeMultipartUploadAsync(const models::CompleteMultipartUploadRequest& request,
                                                    const CompleteMultipartUploadAsyncCallback& callback,
                                                    const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(UploadId);

    auto input = transform::fromCompleteMultipartUpload(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toCompleteMultipartUpload(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::uploadPartCopyAsync(const models::UploadPartCopyRequest& request,
                                          const UploadPartCopyAsyncCallback& callback,
                                          const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(PartNumber);
    requiredFieldAsync(UploadId);
    requiredFieldsOrAsync(SourceKey, CopySource);

    auto input = transform::fromUploadPartCopy(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toUploadPartCopy(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::abortMultipartUploadAsync(const models::AbortMultipartUploadRequest& request,
                                                const AbortMultipartUploadAsyncCallback& callback,
                                                const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(UploadId);

    auto input = transform::fromAbortMultipartUpload(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toAbortMultipartUpload(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::listMultipartUploadsAsync(const models::ListMultipartUploadsRequest& request,
                                                const ListMultipartUploadsAsyncCallback& callback,
                                                const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromListMultipartUploads(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toListMultipartUploads(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::listPartsAsync(const models::ListPartsRequest& request,
                                     const ListPartsAsyncCallback& callback,
                                     const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(UploadId);

    auto input = transform::fromListParts(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toListParts(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
