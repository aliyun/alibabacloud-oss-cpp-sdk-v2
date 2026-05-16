#pragma once

#include "src/internal/ExecuteMiddleware.h"
#include "TransportExecuteMiddleware.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace alibabacloud {
namespace oss2 {
namespace internal {

class ExecuteStack final {
  public:
    using CreateExecuteMiddleware =
            std::function<std::unique_ptr<ExecuteMiddleware>(std::unique_ptr<ExecuteMiddleware>)>;

    explicit ExecuteStack(std::function<std::unique_ptr<ExecuteMiddleware>()> createTransport)
            : createTransport_(std::move(createTransport)) {}

    ~ExecuteStack() = default;

    void Push(CreateExecuteMiddleware create, const std::string& name) {
        ((void) (name));
        stack_.emplace_back(std::move(create));
    }

    void Apply() {
        resolve();
    }

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request, ExecuteContext& context) {
        return handler_->Execute(request, context);
    }

  private:
    void resolve() {
        auto prev = createTransport_();
        for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
            prev = (*it)(std::move(prev));
        }
        handler_ = std::move(prev);
    }

    std::unique_ptr<ExecuteMiddleware> handler_;
    std::vector<CreateExecuteMiddleware> stack_;
    std::function<std::unique_ptr<ExecuteMiddleware>()> createTransport_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud