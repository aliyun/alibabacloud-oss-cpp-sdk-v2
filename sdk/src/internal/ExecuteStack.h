#pragma once

#include "ExecuteMiddleware.h"
#include "TransportExecuteMiddleware.h"

#include <functional>
#include <memory>
#include <mutex>
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
            : cached_(nullptr), createTransport_(createTransport) {}

    virtual ~ExecuteStack() = default;

    void Push(CreateExecuteMiddleware create, const std::string& name) {
        ((void) (name));
        this->stack_.emplace_back(create);
        this->cached_ = nullptr;
    }

    void Apply() {
        resolve();
    }

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request, ExecuteContext& context) {
        return resolve()->Execute(request, context);
    }

  private:
    ExecuteMiddleware* resolve() {
        if (cached_ == nullptr) {
            std::lock_guard<std::mutex> lock(lock_);
            if (cached_ == nullptr) { // cppcheck-suppress identicalInnerCondition
                auto prev = createTransport_();
                for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
                    prev = (*it)(std::move(prev));
                }
                cached_ = std::move(prev);
            }
        }
        return cached_.get();
    }

    std::unique_ptr<ExecuteMiddleware> cached_;
    std::vector<CreateExecuteMiddleware> stack_;
    std::function<std::unique_ptr<ExecuteMiddleware>()> createTransport_;
    std::mutex lock_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud