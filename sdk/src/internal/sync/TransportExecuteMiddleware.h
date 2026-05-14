
#pragma once

#include "src/internal/ExecuteMiddleware.h"
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
        // Send Request
        auto result = httpTransport_->send(request, context.transportContext);
        if (std::holds_alternative<std::error_code>(result)) {
            context.errorContext.error = std::get<1>(result);
            if (!context.transportContext.errorCode.empty()) {
                context.errorContext.errorFields.emplace("Code", std::move(context.transportContext.errorCode));
            }
            if (!context.transportContext.errorMessage.empty()) {
                context.errorContext.errorFields.emplace("Message", std::move(context.transportContext.errorMessage));
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