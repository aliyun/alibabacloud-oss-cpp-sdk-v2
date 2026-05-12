
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/utils/Executor.h"
#include "src/internal/ClientImpl.h"


namespace alibabacloud {
namespace oss2 {

const static ClientOptionsFns defaultClientOptionsFns = ClientOptionsFns{};

OSSClient::OSSClient(const struct ClientConfiguration& config)
        : client_(std::make_shared<internal::ClientImpl>(config, defaultClientOptionsFns))
        , executor_(config.executor) {}

OSSClient::OSSClient(const struct ClientConfiguration& config, ClientOptionsFns& fns)
        : client_(std::make_shared<internal::ClientImpl>(config, fns))
        , executor_(config.executor) {}

OperationResult OSSClient::invokeOperation(const OperationInput& input, const OperationOptions* options) {
    return client_->Execute(input, options);
}

void OSSClient::executeTask(std::function<void()> task) {
    executor_->execute(std::move(task));
}

} // namespace oss2
} // namespace alibabacloud
