
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/ByteStreamUtils.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeObjectBasic.h"
#include "src/utils/Utils.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putObjectAsync(const models::PutObjectRequest& request,
                                     const PutObjectAsyncCallback& callback,
                                     const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromPutObject(request);
    if (client_->hasFlag(FeatureFlagsType::AutoDetectMimeType)) {
        if (input.headers.find("Content-Type") == input.headers.end()) {
            // cppcheck-suppress stlFindInsert
            input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
        }
    }

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

    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toPutObject(std::move(std::get<OperationOutput>(result))));
    }, options, &innerOpts);
}

void OSSAsyncClient::copyObjectAsync(const models::CopyObjectRequest& request,
                                      const CopyObjectAsyncCallback& callback,
                                      const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldsOrAsync(SourceKey, CopySource);

    auto input = transform::fromCopyObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toCopyObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getObjectAsync(const models::GetObjectRequest& request,
                                     const GetObjectAsyncCallback& callback,
                                     const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromGetObject(request);

    internal::OperationInnerOptions innerOpts;
    if (request.getSinkFactory().has_value()) {
        innerOpts.sinkFactory = request.getSinkFactory();
    }

    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toGetObject(std::move(std::get<OperationOutput>(result))));
    }, options, &innerOpts);
}

void OSSAsyncClient::appendObjectAsync(const models::AppendObjectRequest& request,
                                        const AppendObjectAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(Position);

    auto input = transform::fromAppendObject(request);
    if (input.headers.find("Content-Type") == input.headers.end()) {
        // cppcheck-suppress stlFindInsert
        input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
    }
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toAppendObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::sealAppendObjectAsync(const models::SealAppendObjectRequest& request,
                                            const SealAppendObjectAsyncCallback& callback,
                                            const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);
    requiredFieldAsync(Position);

    auto input = transform::fromSealAppendObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toSealAppendObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::deleteObjectAsync(const models::DeleteObjectRequest& request,
                                        const DeleteObjectAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromDeleteObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toDeleteObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::deleteMultipleObjectsAsync(const models::DeleteMultipleObjectsRequest& request,
                                                  const DeleteMultipleObjectsAsyncCallback& callback,
                                                  const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredHasFieldAsync(Delete);

    auto input = transform::fromDeleteMultipleObjects(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toDeleteMultipleObjects(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::headObjectAsync(const models::HeadObjectRequest& request,
                                      const HeadObjectAsyncCallback& callback,
                                      const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromHeadObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toHeadObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getObjectMetaAsync(const models::GetObjectMetaRequest& request,
                                          const GetObjectMetaAsyncCallback& callback,
                                          const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromGetObjectMeta(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toGetObjectMeta(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::restoreObjectAsync(const models::RestoreObjectRequest& request,
                                          const RestoreObjectAsyncCallback& callback,
                                          const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromRestoreObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toRestoreObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::cleanRestoredObjectAsync(const models::CleanRestoredObjectRequest& request,
                                                const CleanRestoredObjectAsyncCallback& callback,
                                                const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromCleanRestoredObject(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toCleanRestoredObject(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
