
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/internal/ClientImpl.h"


namespace alibabacloud {
namespace oss2 {

const static ClientOptionsFns defaultClientOptionsFns = ClientOptionsFns{};

OSSClient::OSSClient(const struct ClientConfiguration& config)
        : client_(std::make_shared<internal::ClientImpl>(config, defaultClientOptionsFns)) {}

OSSClient::OSSClient(const struct ClientConfiguration& config, ClientOptionsFns& fns)
        : client_(std::make_shared<internal::ClientImpl>(config, fns)) {}

OperationResult OSSClient::invokeOperation(const OperationInput& input, const OperationOptions* options) {
    return client_->Execute(input, options);
}


} // namespace oss2
} // namespace alibabacloud
