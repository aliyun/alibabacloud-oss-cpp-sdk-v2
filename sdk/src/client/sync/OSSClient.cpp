
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/internal/sync/ClientImpl.h"


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

bool OSSClient::hasExecutor() const {
    return client_->hasExecutor();
}

void OSSClient::executeTask(std::function<void()> task) {
    client_->executeTask(std::move(task));
}

} // namespace oss2
} // namespace alibabacloud
