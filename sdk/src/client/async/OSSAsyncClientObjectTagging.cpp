
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeObjectTagging.h"
#include "OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putObjectTaggingAsync(const models::PutObjectTaggingRequest& request,
                                            const PutObjectTaggingAsyncCallback& callback,
                                            const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromPutObjectTagging(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toPutObjectTagging(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::getObjectTaggingAsync(const models::GetObjectTaggingRequest& request,
                                            const GetObjectTaggingAsyncCallback& callback,
                                            const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromGetObjectTagging(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toGetObjectTagging(std::move(std::get<OperationOutput>(result))));
    }, options);
}

void OSSAsyncClient::deleteObjectTaggingAsync(const models::DeleteObjectTaggingRequest& request,
                                                const DeleteObjectTaggingAsyncCallback& callback,
                                                const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredFieldAsync(Key);

    auto input = transform::fromDeleteObjectTagging(request);
    client_->ExecuteAsync(input, [callback](OperationResult result) {
        if (std::holds_alternative<OperationError>(result)) {
            callback(std::get<OperationError>(std::move(result)));
            return;
        }
        callback(transform::toDeleteObjectTagging(std::move(std::get<OperationOutput>(result))));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
