
#pragma once

#include "AsyncExecuteMiddleware.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class OnFinishedAsyncMiddleware final : public AsyncExecuteMiddleware {
  public:
    using OnFinished = std::function<void(const std::shared_ptr<AsyncExecuteState>&)>;

    explicit OnFinishedAsyncMiddleware(OnFinished onFinished)
            : onFinished_(std::move(onFinished)) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>&) override {}

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        onFinished_(state);
    }

  private:
    OnFinished onFinished_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
