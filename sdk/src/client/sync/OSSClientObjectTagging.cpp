
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeObjectTagging.h"
#include "OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


PutObjectTaggingOutcome OSSClient::putObjectTagging(const models::PutObjectTaggingRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromPutObjectTagging(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutObjectTagging(std::move(std::get<OperationOutput>(result)));
}

GetObjectTaggingOutcome OSSClient::getObjectTagging(const models::GetObjectTaggingRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObjectTagging(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetObjectTagging(std::move(std::get<OperationOutput>(result)));
}

DeleteObjectTaggingOutcome OSSClient::deleteObjectTagging(const models::DeleteObjectTaggingRequest& request,
                                                          const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromDeleteObjectTagging(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toDeleteObjectTagging(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
