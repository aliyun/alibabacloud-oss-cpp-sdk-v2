
#pragma once

#include "src/internal/ClientImplBase.h"
#include "ExecuteStack.h"

#include <functional>
#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace internal {

struct PresignInnerOutput {
    std::string url;
    std::string method;
    std::time_t expiration;
    HeaderCollection signedHeaders;
};

using PresignInnerResult = std::variant<PresignInnerOutput, OperationError>;

class ClientImpl : public ClientImplBase {
  public:
    explicit ClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns);
    ~ClientImpl() override = default;

    OperationResult Execute(const OperationInput& input, const OperationOptions* opts = nullptr,
                            const OperationInnerOptions* innerOpts = nullptr);

    PresignInnerResult Presign(const OperationInput& input, const OperationOptions* opts = nullptr);

    bool hasExecutor() const;
    void executeTask(std::function<void()> task);

  private:
    std::shared_ptr<ExecuteStack> executeStack_;
    std::shared_ptr<Executor> executor_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
