
#pragma once

#include "src/internal/ExecuteMiddleware.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"

#include <chrono>
#include <memory>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace internal {

enum class ResponseAction {
    Stop,
    Continue,
};

struct AsyncExecuteState {
    std::string opName;
    OperationCallback callback;
    ExecuteContext context;

    std::unique_ptr<RequestMessage> request;
    std::unique_ptr<ResponseMessage> response;

    ResponseAction action{ResponseAction::Stop};
    std::chrono::milliseconds retryDelay{0};

    long retries{0};
    std::time_t signTime{};
    std::time_t expiration{};
};

class AsyncExecuteMiddleware {
  public:
    virtual ~AsyncExecuteMiddleware() = default;

    void setPrev(AsyncExecuteMiddleware* parent) { prev_ = parent; }

    virtual void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) = 0;
    virtual void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) = 0;

  protected:
    AsyncExecuteMiddleware* prev_{nullptr};
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
