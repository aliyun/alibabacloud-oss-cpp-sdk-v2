
#pragma once

#include "src/internal/ExecuteMiddleware.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

namespace alibabacloud {
namespace oss2 {

namespace internal {
class TransportExecuteMiddleware final : public ExecuteMiddleware {
  public:
    TransportExecuteMiddleware(std::shared_ptr<HttpTransport> httpTransport)
            : httpTransport_(std::move(httpTransport)) {}

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request,
                                             ExecuteContext& context) override {
        if (context.transportContext.cancellationToken.has_value() &&
            context.transportContext.cancellationToken->isCanceled()) {
            context.errorContext.error = make_error_code(TransportErrorCode::Canceled);
            context.errorContext.errorFields.emplace("Code", "RequestCanceled");
            context.errorContext.errorFields.emplace("Message", "Request canceled by CancellationToken");
            return nullptr;
        }

        auto result = httpTransport_->send(request, context.transportContext);
        if (std::holds_alternative<TransportError>(result)) {
            auto& te = std::get<TransportError>(result);
            context.errorContext.error = te.error;
            if (!te.errorCode.empty()) {
                context.errorContext.errorFields.emplace("Code", std::move(te.errorCode));
            }
            if (!te.errorMessage.empty()) {
                context.errorContext.errorFields.emplace("Message", std::move(te.errorMessage));
            }
            return nullptr;
        }

        // cppcheck-suppress returnStdMoveLocal
        return std::move(std::get<0>(result));
    }

  private:
    std::shared_ptr<HttpTransport> httpTransport_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud