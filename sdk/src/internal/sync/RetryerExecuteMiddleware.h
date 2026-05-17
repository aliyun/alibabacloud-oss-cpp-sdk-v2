
#pragma once

#include "src/internal/ExecuteMiddleware.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/retry/Retryer.h"


#include <atomic>
#include <condition_variable>
#include <mutex>

namespace alibabacloud {
namespace oss2 {
namespace internal {

class RetryerExecuteMiddleware final : public ExecuteMiddleware {
  public:
    RetryerExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler, std::shared_ptr<Retryer> retryer)
            : nextHandler_(std::move(nextHandler)), retryer_(std::move(retryer)), disable_(false) {}

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
            if (context.transportContext.ostreamFactory.has_value() &&
                context.transportContext.ostreamFactory.value().isOneShot) {
                break;
            }

            if (!retryer_->isErrorRetryable(context.errorContext.error)) {
                break;
            }

            // delay
            auto delay = retryer_->calcDelayTime(context.errorContext.error, retries + 1);
            if (waitForRetry(delay)) {
                // cancel, and break
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


  public:
    void disable() {
        disable_ = true;
        requestSignal_.notify_all();
    }

    void enable() {
        disable_ = false;
    }

    bool waitForRetry(std::chrono::milliseconds milliseconds) {
        if (milliseconds.count() == 0) {
            return false;
        }
        std::unique_lock<std::mutex> lck(requestLock_);
        return requestSignal_.wait_for(lck, milliseconds, [this]() -> bool { return disable_.load() == true; });
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
    std::shared_ptr<Retryer> retryer_;
    std::atomic<bool> disable_;
    std::mutex requestLock_;
    std::condition_variable requestSignal_;
};
} // namespace internal

} // namespace oss2
} // namespace alibabacloud