
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeBucketVersioning.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putBucketVersioningAsync(const models::PutBucketVersioningRequest& request,
                                              const PutBucketVersioningAsyncCallback& callback,
                                              const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromPutBucketVersioning(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toPutBucketVersioning(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getBucketVersioningAsync(const models::GetBucketVersioningRequest& request,
                                              const GetBucketVersioningAsyncCallback& callback,
                                              const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketVersioning(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toGetBucketVersioning(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::listObjectVersionsAsync(const models::ListObjectVersionsRequest& request,
                                              const ListObjectVersionsAsyncCallback& callback,
                                              const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromListObjectVersions(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toListObjectVersions(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
