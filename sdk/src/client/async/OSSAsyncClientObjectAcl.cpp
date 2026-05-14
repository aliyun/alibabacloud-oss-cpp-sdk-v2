
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeObjectAcl.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putObjectAclAsync(const models::PutObjectAclRequest& request,
                                        const PutObjectAclAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromPutObjectAcl(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toPutObjectAcl(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getObjectAclAsync(const models::GetObjectAclRequest& request,
                                        const GetObjectAclAsyncCallback& callback,
                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromGetObjectAcl(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toGetObjectAcl(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
