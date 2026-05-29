#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/internal/async/AsyncClientImpl.h"

namespace alibabacloud {
namespace oss2 {

OSSAsyncClient::OSSAsyncClient(const struct ClientConfiguration& config)
    : client_(std::make_shared<internal::AsyncClientImpl>(config, ClientOptionsFns{})) {}

OSSAsyncClient::OSSAsyncClient(const struct ClientConfiguration& config, ClientOptionsFns& fns)
    : client_(std::make_shared<internal::AsyncClientImpl>(config, fns)) {}

OSSAsyncClient::~OSSAsyncClient() = default;

void OSSAsyncClient::invokeOperationAsync(const OperationInput& input, const OperationCallback& callback,
                                          const OperationOptions* options) {
    client_->ExecuteAsync(input, callback, options);
}

} // namespace oss2
} // namespace alibabacloud
