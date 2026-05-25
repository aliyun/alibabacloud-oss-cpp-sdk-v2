
#pragma once

#include "src/internal/ExecuteMiddleware.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {
namespace internal {

using ClientWaitForFn = std::function<bool(std::chrono::milliseconds)>;

class RetryerExecuteMiddleware final : public ExecuteMiddleware {
  public:
    RetryerExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler,
                             std::shared_ptr<Retryer> retryer,
                             ClientWaitForFn clientWaitFor)
            : nextHandler_(std::move(nextHandler)),
              retryer_(std::move(retryer)),
              clientWaitFor_(std::move(clientWaitFor)) {}

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request,
                                             ExecuteContext& context) override {
        std::unique_ptr<ResponseMessage> response = nullptr;
        long attempts = context.retryMaxAttempts;
        auto signTime = context.signingContext.signTimeInEpoch;
        auto expiration = context.signingContext.expirationInEpoch;

        for (long retries = 0;; retries++) {
            response = nextHandler_->Execute(request, context);

            if (!context.errorContext.error) {
                break;
            }

            if (retries + 1 >= attempts) {
                break;
            }

            if (context.errorContext.error == ErrorCondition::Canceled) {
                break;
            }

            // request.body().isReplayable()
            if (request->body != nullptr && request->body->isOneShot()) {
                break;
            }

            // response.body().isReplayable()
            if (context.transportContext.sinkFactory.has_value() &&
                context.transportContext.sinkFactory.value().isOneShot) {
                break;
            }

            if (!retryer_->isErrorRetryable(context.errorContext.error)) {
                break;
            }

            // delay
            auto delay = retryer_->calcDelayTime(context.errorContext.error, retries + 1);
            if (waitForRetry(delay, context.transportContext.cancellationToken)) {
                context.errorContext.error = make_error_code(ClientErrorCode::OperationCanceled);
                break;
            }

            // reset to init state
            context.errorContext.errorFields.clear();
            context.errorContext.snapshot = "";
            context.errorContext.error = std::error_code();

            // reset signing time
            context.signingContext.signTimeInEpoch = signTime;
            context.signingContext.expirationInEpoch = expiration;
        }

        return response;
    }

  private:
    static constexpr auto kNotifyThreshold = std::chrono::milliseconds(200);

    bool waitForRetry(std::chrono::milliseconds delay, const std::optional<CancellationToken>& token) {
        if (delay.count() == 0) return false;

        if (delay < kNotifyThreshold || !token.has_value() || !token->canBeCanceled()) {
            return clientWaitFor_(delay) || (token.has_value() && token->isCanceled());
        }

        return token->waitFor(delay);
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
    std::shared_ptr<Retryer> retryer_;
    ClientWaitForFn clientWaitFor_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud