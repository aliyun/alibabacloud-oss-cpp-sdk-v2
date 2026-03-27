
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeObjectAcl.h"

namespace alibabacloud {
namespace oss2 {

#define requiredFiled(field)                                                                             \
    do {                                                                                                 \
        if (request.get##field().empty()) {                                                              \
            return OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                       \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field ""}}); \
        }                                                                                                \
    } while (false)


// Object Acl
PutObjectAclOutcome OSSClient::putObjectAcl(const models::PutObjectAclRequest& request,
                                            const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);

    auto input = transform::fromPutObjectAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutObjectAcl(std::move(std::get<OperationOutput>(result)));
}

GetObjectAclOutcome OSSClient::getObjectAcl(const models::GetObjectAclRequest& request,
                                            const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);

    auto input = transform::fromGetObjectAcl(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetObjectAcl(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
