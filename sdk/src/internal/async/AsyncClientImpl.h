
#pragma once

#include "src/internal/ClientImplBase.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {

class AsyncHttpTransport;
class ScheduledExecutor;

namespace internal {

class AsyncExecuteStack;
struct AsyncExecuteState;

class AsyncClientImpl : public ClientImplBase {
  public:
    explicit AsyncClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns);
    ~AsyncClientImpl() override;

    void ExecuteAsync(const OperationInput& input,
                      OperationCallback callback,
                      const OperationOptions* opts = nullptr,
                      const OperationInnerOptions* innerOpts = nullptr);

  private:
    void onOperationFinished(const std::shared_ptr<AsyncExecuteState>& state);
    std::shared_ptr<ScheduledExecutor> scheduler_;
    std::shared_ptr<AsyncExecuteStack> stack_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
