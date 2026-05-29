
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeService.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::listBucketsAsync(const models::ListBucketsRequest& request,
                                      const ListBucketsAsyncCallback& callback, const OperationOptions* options) {
    auto input = transform::fromListBuckets(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toListBuckets(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

} // namespace oss2
} // namespace alibabacloud
