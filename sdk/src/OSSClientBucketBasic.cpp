
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeBucketBasic.h"
#include "src/OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


GetBucketStatOutcome OSSClient::getBucketStat(const models::GetBucketStatRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketStat(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetBucketStat(std::move(std::get<OperationOutput>(result)));
}

PutBucketOutcome OSSClient::putBucket(const models::PutBucketRequest& request, const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromPutBucket(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutBucket(std::move(std::get<OperationOutput>(result)));
}

DeleteBucketOutcome OSSClient::deleteBucket(const models::DeleteBucketRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromDeleteBucket(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toDeleteBucket(std::move(std::get<OperationOutput>(result)));
}

ListObjectsOutcome OSSClient::listObjects(const models::ListObjectsRequest& request, const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromListObjects(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toListObjects(std::move(std::get<OperationOutput>(result)));
}

ListObjectsV2Outcome OSSClient::listObjectsV2(const models::ListObjectsV2Request& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromListObjectsV2(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toListObjectsV2(std::move(std::get<OperationOutput>(result)));
}

GetBucketInfoOutcome OSSClient::getBucketInfo(const models::GetBucketInfoRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketInfo(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetBucketInfo(std::move(std::get<OperationOutput>(result)));
}

GetBucketLocationOutcome OSSClient::getBucketLocation(const models::GetBucketLocationRequest& request,
                                                      const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketLocation(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetBucketLocation(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
