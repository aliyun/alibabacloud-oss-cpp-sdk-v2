
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeBucketVersioning.h"
#include "OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


PutBucketVersioningOutcome OSSClient::putBucketVersioning(const models::PutBucketVersioningRequest& request,
                                                          const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromPutBucketVersioning(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutBucketVersioning(std::move(std::get<OperationOutput>(result)));
}

GetBucketVersioningOutcome OSSClient::getBucketVersioning(const models::GetBucketVersioningRequest& request,
                                                          const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketVersioning(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetBucketVersioning(std::move(std::get<OperationOutput>(result)));
}

ListObjectVersionsOutcome OSSClient::listObjectVersions(const models::ListObjectVersionsRequest& request,
                                                        const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromListObjectVersions(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toListObjectVersions(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
