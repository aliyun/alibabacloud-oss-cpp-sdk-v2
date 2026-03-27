
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeObjectSymlink.h"

namespace alibabacloud {
namespace oss2 {

#define requiredFiled(field)                                                                             \
    do {                                                                                                 \
        if (request.get##field().empty()) {                                                              \
            return OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                       \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Miss filed " #field ""}}); \
        }                                                                                                \
    } while (false)


PutSymlinkOutcome OSSClient::putSymlink(const models::PutSymlinkRequest& request, const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);

    auto input = transform::fromPutSymlink(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toPutSymlink(std::move(std::get<OperationOutput>(result)));
}

GetSymlinkOutcome OSSClient::getSymlink(const models::GetSymlinkRequest& request, const OperationOptions* options) {
    requiredFiled(Bucket);
    requiredFiled(Key);

    auto input = transform::fromGetSymlink(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toGetSymlink(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
