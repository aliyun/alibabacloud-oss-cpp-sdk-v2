
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/transform/SerdeRegion.h"

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::describeRegionsAsync(const models::DescribeRegionsRequest& request,
                                          const DescribeRegionsAsyncCallback& callback,
                                          const OperationOptions* options) {
    auto input = transform::fromDescribeRegions(request);
    client_->ExecuteAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toDescribeRegions(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

} // namespace oss2
} // namespace alibabacloud
