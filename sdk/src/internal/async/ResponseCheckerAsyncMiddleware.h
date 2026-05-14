
#pragma once

#include "AsyncExecuteMiddleware.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class ResponseCheckerAsyncMiddleware final : public AsyncExecuteMiddleware {
  public:
    explicit ResponseCheckerAsyncMiddleware(std::unique_ptr<AsyncExecuteMiddleware> next = nullptr)
            : next_(std::move(next)) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) override {
        next_->handleRequest(state);
    }

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        if (!std::holds_alternative<std::error_code>(state->result)) {
            auto& response = std::get<std::unique_ptr<ResponseMessage>>(state->result);
            if (response != nullptr && !state->context.errorContext.error) {
                for (const auto& fn : state->context.onResponseMessage) {
                    if (!fn(response, state->context)) {
                        break;
                    }
                }
            }
        }

        prev_->handleResponse(state);
    }

  private:
    std::unique_ptr<AsyncExecuteMiddleware> next_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
