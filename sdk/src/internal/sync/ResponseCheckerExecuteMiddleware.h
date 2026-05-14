
#pragma once

#include "src/internal/ExecuteMiddleware.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class ResponseCheckerExecuteMiddleware final : public ExecuteMiddleware {
  public:
    ResponseCheckerExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler)
            : nextHandler_(std::move(nextHandler)) {}

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request,
                                             ExecuteContext& context) override {
        auto response = nextHandler_->Execute(request, context);

        if (response != nullptr && !context.errorContext.error) {
            for (const auto& fn : context.onResponseMessage) {
                if (!fn(response, context)) {
                    break;
                }
            }
        }

        return response;
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
};
} // namespace internal

} // namespace oss2
} // namespace alibabacloud