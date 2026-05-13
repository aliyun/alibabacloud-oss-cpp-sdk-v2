
#pragma once

#include "AsyncExecuteMiddleware.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class TransportAsyncMiddleware final : public AsyncExecuteMiddleware {
  public:
    explicit TransportAsyncMiddleware(AsyncHttpTransport* transport)
            : transport_(transport) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) override {
        RequestContext transportCtx;
        transportCtx.ostreamFactory = state->context.transportContext.ostreamFactory;

        auto req = std::move(state->request);
        auto self = this;
        auto s = state;
        transport_->sendAsync(std::move(req), std::move(transportCtx),
            [self, s](ResponseResult result, std::unique_ptr<RequestMessage> request,
                       RequestContext context) mutable {
                s->request = std::move(request);
                s->context.transportContext = std::move(context);

                if (std::holds_alternative<std::error_code>(result)) {
                    s->context.errorContext.error = std::get<std::error_code>(result);
                    if (!s->context.transportContext.errorCode.empty()) {
                        s->context.errorContext.errorFields.emplace(
                            "Code", std::move(s->context.transportContext.errorCode));
                    }
                    if (!s->context.transportContext.errorMessage.empty()) {
                        s->context.errorContext.errorFields.emplace(
                            "Message", std::move(s->context.transportContext.errorMessage));
                    }
                } else {
                    s->result = std::move(result);
                }

                self->handleResponse(s);
            });
    }

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        prev_->handleResponse(state);
    }

  private:
    AsyncHttpTransport* transport_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
