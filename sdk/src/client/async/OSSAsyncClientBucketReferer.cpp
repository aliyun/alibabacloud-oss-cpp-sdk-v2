
#include "OSSAsyncClientUtils.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeBucketReferer.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putBucketRefererAsync(const models::PutBucketRefererRequest& request,
                                           const PutBucketRefererAsyncCallback& callback,
                                           const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromPutBucketReferer(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toPutBucketReferer(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncClient::getBucketRefererAsync(const models::GetBucketRefererRequest& request,
                                           const GetBucketRefererAsyncCallback& callback,
                                           const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketReferer(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toGetBucketReferer(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

} // namespace oss2
} // namespace alibabacloud
