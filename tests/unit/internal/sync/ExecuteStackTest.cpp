#include <gtest/gtest.h>

#include "src/internal/sync/ExecuteStack.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class Test1ExecuteMiddleware : public ExecuteMiddleware {
  public:
    Test1ExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler) : nextHandler_(std::move(nextHandler)) {}
    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request, ExecuteContext& context) {
        std::cout << "Test1ExecuteMiddleware" << std::endl;
        return nextHandler_->Execute(request, context);
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
};

class Test2ExecuteMiddleware : public ExecuteMiddleware {
  public:
    Test2ExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler) : nextHandler_(std::move(nextHandler)) {}
    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request, ExecuteContext& context) {
        std::cout << "Test2ExecuteMiddleware" << std::endl;
        return nextHandler_->Execute(request, context);
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
};

class MockHttpTransport : public HttpTransport {
  public:
    MockHttpTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override {
        std::cout << "MockHttpTransport" << std::endl;
        return std::make_unique<ResponseMessage>();
    }

    std::string getName() const override {
        return "MockHttpTransport";
    }
};

TEST(ExecuteStackTest, ExecuteStackCtor) {
    auto httpTransport = std::make_shared<MockHttpTransport>();
    auto fn = [httpTransport]() -> std::unique_ptr<ExecuteMiddleware> {
        return std::make_unique<TransportExecuteMiddleware>(httpTransport);
    };
    auto stack = std::make_unique<ExecuteStack>(fn);

    stack->Push(
            [](std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<Test1ExecuteMiddleware>(std::move(handle));
            },
            "test1");

    stack->Push(
            [](std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<Test2ExecuteMiddleware>(std::move(handle));
            },
            "test2");

    stack->Apply();

    auto requestMessage = std::make_unique<RequestMessage>();
    auto executeContext = ExecuteContext();
    stack->Execute(requestMessage, executeContext);
}


} // namespace internal
} // namespace oss2
} // namespace alibabacloud