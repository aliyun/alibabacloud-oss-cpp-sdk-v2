
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
        auto req = std::move(state->request);
        auto self = this;
        auto s = state;
        transport_->sendAsync(std::move(req), s->context.transportContext,
            [self, s](ResponseResult result, std::unique_ptr<RequestMessage> request) mutable {
                s->request = std::move(request);

                if (std::holds_alternative<TransportError>(result)) {
                    auto& te = std::get<TransportError>(result);
                    s->context.errorContext.error = te.error;
                    if (!te.errorCode.empty()) {
                        s->context.errorContext.errorFields.emplace(
                            "Code", std::move(te.errorCode));
                    }
                    if (!te.errorMessage.empty()) {
                        s->context.errorContext.errorFields.emplace(
                            "Message", std::move(te.errorMessage));
                    }
                } else {
                    s->response = std::move(std::get<std::unique_ptr<ResponseMessage>>(result));
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
