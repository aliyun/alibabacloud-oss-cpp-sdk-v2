
#pragma once

#include "ClientImplBase.h"
#include "ExecuteStack.h"

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

  private:
    std::shared_ptr<ExecuteStack> executeStack_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
