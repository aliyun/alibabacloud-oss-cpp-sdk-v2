
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/ClientImpl.h"
#include "src/transform/SerdeService.h"

namespace alibabacloud {
namespace oss2 {

ListBucketsOutcome OSSClient::listBuckets(const models::ListBucketsRequest& request, const OperationOptions* options) {
    auto input = transform::fromListBuckets(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return std::get<OperationError>(result);
    }
    return transform::toListBuckets(std::move(std::get<OperationOutput>(result)));
}


} // namespace oss2
} // namespace alibabacloud
