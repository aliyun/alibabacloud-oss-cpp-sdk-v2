
#include "AsyncExecuteStack.h"
#include "TransportAsyncMiddleware.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

AsyncExecuteStack::AsyncExecuteStack(std::shared_ptr<AsyncHttpTransport> transport, OnFinished onFinished)
    : sentinel_(std::move(onFinished)), transport_(std::move(transport)) {}

AsyncExecuteStack::~AsyncExecuteStack() = default;

void AsyncExecuteStack::Push(CreateMiddleware create, const std::string& name) {
    ((void) (name));
    stack_.emplace_back(std::move(create));
}

void AsyncExecuteStack::Apply() {
    resolve();
}

void AsyncExecuteStack::resolve() {
    // Build the chain inside-out. Each factory takes ownership of "prev" as its
    // next_ member, so "inner" (the raw pointer we saved) remains valid and is
    // now owned by the newly created outer wrapper. We then link inner->prev_
    // back to that outer wrapper for the response (right-to-left) path.
    std::unique_ptr<AsyncExecuteMiddleware> prev = std::make_unique<TransportAsyncMiddleware>(transport_.get());
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        auto* inner = prev.get();
        prev = (*it)(std::move(prev));
        inner->setPrev(prev.get());
    }
    handler_ = std::move(prev);
    handler_->setPrev(&sentinel_);
}

void AsyncExecuteStack::executeAsync(const std::shared_ptr<AsyncExecuteState>& state) {
    handler_->handleRequest(state);
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
