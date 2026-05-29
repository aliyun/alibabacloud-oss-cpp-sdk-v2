
#include "OSSClientUtils.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeBucketAcl.h"

namespace alibabacloud {
namespace oss2 {


PutBucketAclOutcome OSSClient::putBucketAcl(const models::PutBucketAclRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromPutBucketAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutBucketAcl(std::move(std::get<OperationOutput>(result)));
}

GetBucketAclOutcome OSSClient::getBucketAcl(const models::GetBucketAclRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);

    auto input = transform::fromGetBucketAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetBucketAcl(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
