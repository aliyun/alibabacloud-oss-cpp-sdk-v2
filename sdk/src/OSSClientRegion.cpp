
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeRegion.h"

namespace alibabacloud {
namespace oss2 {

DescribeRegionsOutcome OSSClient::describeRegions(const models::DescribeRegionsRequest& request,
                                                  const OperationOptions* options) {
    auto input = transform::fromDescribeRegions(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toDescribeRegions(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
