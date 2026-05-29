
#include "OSSAsyncClientUtils.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeBucketBasic.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::getBucketStatAsync(const models::GetBucketStatRequest& request,
                                        const GetBucketStatAsyncCallback& callback, const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketStat(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toGetBucketStat(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::putBucketAsync(const models::PutBucketRequest& request, const PutBucketAsyncCallback& callback,
                                    const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromPutBucket(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toPutBucket(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::deleteBucketAsync(const models::DeleteBucketRequest& request,
                                       const DeleteBucketAsyncCallback& callback, const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromDeleteBucket(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toDeleteBucket(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::listObjectsAsync(const models::ListObjectsRequest& request,
                                      const ListObjectsAsyncCallback& callback, const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromListObjects(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toListObjects(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::listObjectsV2Async(const models::ListObjectsV2Request& request,
                                        const ListObjectsV2AsyncCallback& callback, const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromListObjectsV2(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toListObjectsV2(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::getBucketInfoAsync(const models::GetBucketInfoRequest& request,
                                        const GetBucketInfoAsyncCallback& callback, const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketInfo(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toGetBucketInfo(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::getBucketLocationAsync(const models::GetBucketLocationRequest& request,
                                            const GetBucketLocationAsyncCallback& callback,
                                            const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketLocation(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toGetBucketLocation(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

} // namespace oss2
} // namespace alibabacloud
