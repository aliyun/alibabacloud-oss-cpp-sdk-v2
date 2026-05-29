
#pragma once

#include "ExecuteStack.h"
#include "src/internal/ClientImplBase.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace alibabacloud {
namespace oss2 {
namespace internal {

struct DisableState {
    std::atomic<bool> flag{false};
    std::mutex mu;
    std::condition_variable cv;
};

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

    void disableRequest();
    void enableRequest();

  private:
    std::shared_ptr<DisableState> disableState_;
    std::shared_ptr<ExecuteStack> executeStack_;
    std::shared_ptr<Executor> executor_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
