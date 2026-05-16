
#pragma once

#include "AsyncExecuteMiddleware.h"
#include "OnFinishedAsyncMiddleware.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace internal {

class AsyncExecuteStack {
  public:
    using CreateMiddleware =
            std::function<std::unique_ptr<AsyncExecuteMiddleware>(std::unique_ptr<AsyncExecuteMiddleware>)>;
    using OnFinished = OnFinishedAsyncMiddleware::OnFinished;

    AsyncExecuteStack(std::shared_ptr<AsyncHttpTransport> transport, OnFinished onFinished);
    ~AsyncExecuteStack();

    void Push(CreateMiddleware create, const std::string& name);
    void Apply();

    void executeAsync(const std::shared_ptr<AsyncExecuteState>& state);

  private:
    void resolve();

    std::vector<CreateMiddleware> stack_;
    OnFinishedAsyncMiddleware sentinel_;
    std::unique_ptr<AsyncExecuteMiddleware> handler_;
    std::shared_ptr<AsyncHttpTransport> transport_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
