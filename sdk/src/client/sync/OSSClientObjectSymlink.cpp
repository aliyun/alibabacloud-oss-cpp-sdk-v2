
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeObjectSymlink.h"
#include "OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {


PutSymlinkOutcome OSSClient::putSymlink(const models::PutSymlinkRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromPutSymlink(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutSymlink(std::move(std::get<OperationOutput>(result)));
}

GetSymlinkOutcome OSSClient::getSymlink(const models::GetSymlinkRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetSymlink(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetSymlink(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
