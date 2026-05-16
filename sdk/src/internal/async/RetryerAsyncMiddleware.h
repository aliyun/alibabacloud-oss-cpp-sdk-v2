
#pragma once

#include "AsyncExecuteMiddleware.h"
#include "alibabacloud/oss2/retry/Retryer.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class RetryerAsyncMiddleware final : public AsyncExecuteMiddleware {
  public:
    RetryerAsyncMiddleware(std::unique_ptr<AsyncExecuteMiddleware> next,
                           std::shared_ptr<Retryer> retryer)
            : next_(std::move(next)),
              retryer_(std::move(retryer)) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) override {
        state->action = ResponseAction::Stop;
        state->retryDelay = std::chrono::milliseconds(0);
        next_->handleRequest(state);
    }

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        if (state->context.errorContext.error) {
            long attempts = state->context.retryMaxAttempts;
            if (state->retries + 1 < attempts) {
                bool canRetry = true;
                if (state->request->body != nullptr && state->request->body->isOneShot()) {
                    canRetry = false;
                }
                if (state->context.transportContext.ostreamFactory.has_value() &&
                    state->context.transportContext.ostreamFactory.value().isOneShot) {
                    canRetry = false;
                }
                if (canRetry && retryer_->isErrorRetryable(state->context.errorContext.error)) {
                    state->retryDelay = retryer_->calcDelayTime(
                            state->context.errorContext.error, state->retries + 1);
                    state->retries++;

                    state->context.errorContext.errorFields.clear();
                    state->context.errorContext.snapshot.clear();
                    state->context.errorContext.error = std::error_code();
                    state->context.signingContext.signTimeInEpoch = state->signTime;
                    state->context.signingContext.expirationInEpoch = state->expiration;

                    state->action = ResponseAction::Continue;
                }
            }
        }

        prev_->handleResponse(state);
    }

  private:
    std::unique_ptr<AsyncExecuteMiddleware> next_;
    std::shared_ptr<Retryer> retryer_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
