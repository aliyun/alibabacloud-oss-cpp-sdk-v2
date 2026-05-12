
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeObjectBasic.h"
#include "src/utils/Utils.h"
#include "src/OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


// Object Basic
PutObjectOutcome OSSClient::putObject(const models::PutObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromPutObject(request);
    if (client_->hasFlag(FeatureFlagsType::AutoDetectMimeType)) {
        if (input.headers.find("Content-Type") == input.headers.end()) {
            // cppcheck-suppress stlFindInsert
            input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
        }
    }
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutObject(std::move(std::get<OperationOutput>(result)));
}

CopyObjectOutcome OSSClient::copyObject(const models::CopyObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredFieldsOr(SourceKey, CopySource);

    auto input = transform::fromCopyObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toCopyObject(std::move(std::get<OperationOutput>(result)));
}

GetObjectOutcome OSSClient::getObject(const models::GetObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetObject(std::move(std::get<OperationOutput>(result)));
}

AppendObjectOutcome OSSClient::appendObject(const models::AppendObjectRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Position);

    auto input = transform::fromAppendObject(request);
    if (input.headers.find("Content-Type") == input.headers.end()) {
        // cppcheck-suppress stlFindInsert
        input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
    }

    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toAppendObject(std::move(std::get<OperationOutput>(result)));
}

SealAppendObjectOutcome OSSClient::sealAppendObject(const models::SealAppendObjectRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Position);

    auto input = transform::fromSealAppendObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toSealAppendObject(std::move(std::get<OperationOutput>(result)));
}

DeleteObjectOutcome OSSClient::deleteObject(const models::DeleteObjectRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromDeleteObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toDeleteObject(std::move(std::get<OperationOutput>(result)));
}

DeleteMultipleObjectsOutcome OSSClient::deleteMultipleObjects(const models::DeleteMultipleObjectsRequest& request,
                                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredHasField(Delete);

    auto input = transform::fromDeleteMultipleObjects(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toDeleteMultipleObjects(std::move(std::get<OperationOutput>(result)));
}

HeadObjectOutcome OSSClient::headObject(const models::HeadObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromHeadObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toHeadObject(std::move(std::get<OperationOutput>(result)));
}

GetObjectMetaOutcome OSSClient::getObjectMeta(const models::GetObjectMetaRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObjectMeta(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetObjectMeta(std::move(std::get<OperationOutput>(result)));
}

RestoreObjectOutcome OSSClient::restoreObject(const models::RestoreObjectRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromRestoreObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toRestoreObject(std::move(std::get<OperationOutput>(result)));
}

CleanRestoredObjectOutcome OSSClient::cleanRestoredObject(const models::CleanRestoredObjectRequest& request,
                                                          const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromCleanRestoredObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toCleanRestoredObject(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
