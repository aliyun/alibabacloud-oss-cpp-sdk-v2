
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeBucketReferer.h"
#include "src/OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


PutBucketRefererOutcome OSSClient::putBucketReferer(const models::PutBucketRefererRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromPutBucketReferer(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutBucketReferer(std::move(std::get<OperationOutput>(result)));
}

GetBucketRefererOutcome OSSClient::getBucketReferer(const models::GetBucketRefererRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketReferer(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetBucketReferer(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
