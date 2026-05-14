
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeBucketAcl.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putBucketAclAsync(const models::PutBucketAclRequest& request,
                                        const PutBucketAclAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromPutBucketAcl(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toPutBucketAcl(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getBucketAclAsync(const models::GetBucketAclRequest& request,
                                        const GetBucketAclAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);

    auto input = transform::fromGetBucketAcl(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(makeUnexpected(std::get<OperationError>(std::move(result))));
            return;
        }
        callback(transform::toGetBucketAcl(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
