
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeObjectAcl.h"
#include "OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


// Object Acl
PutObjectAclOutcome OSSClient::putObjectAcl(const models::PutObjectAclRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromPutObjectAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutObjectAcl(std::move(std::get<OperationOutput>(result)));
}

GetObjectAclOutcome OSSClient::getObjectAcl(const models::GetObjectAclRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObjectAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetObjectAcl(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
