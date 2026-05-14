
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeObjectSymlink.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putSymlinkAsync(const models::PutSymlinkRequest& request,
                                      const PutSymlinkAsyncCallback& callback,
                                      const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromPutSymlink(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toPutSymlink(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getSymlinkAsync(const models::GetSymlinkRequest& request,
                                      const GetSymlinkAsyncCallback& callback,
                                      const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromGetSymlink(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toGetSymlink(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
