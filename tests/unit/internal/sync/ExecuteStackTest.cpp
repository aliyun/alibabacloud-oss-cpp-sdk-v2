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
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
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


class MetricsMockTransport : public HttpTransport {
  public:
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        auto resp = std::make_unique<ResponseMessage>();
        resp->statusCode = 200;
        resp->metrics = std::make_unique<HttpMetrics>();
        resp->metrics->requestStart = std::chrono::system_clock::now();
        resp->metrics->dnsLookup = std::chrono::microseconds(1500);
        resp->metrics->connect = std::chrono::microseconds(3200);
        resp->metrics->tlsHandshake = std::chrono::microseconds(8500);
        resp->metrics->startTransfer = std::chrono::microseconds(12000);
        resp->metrics->total = std::chrono::microseconds(45000);
        resp->metrics->connectionReused = false;
        return resp;
    }
    std::string getName() const override {
        return "MetricsMockTransport";
    }
};

TEST(ExecuteStackTest, MetricsPropagatesThroughStack) {
    auto httpTransport = std::make_shared<MetricsMockTransport>();
    auto fn = [httpTransport]() -> std::unique_ptr<ExecuteMiddleware> {
        return std::make_unique<TransportExecuteMiddleware>(httpTransport);
    };
    auto stack = std::make_unique<ExecuteStack>(fn);

    stack->Push(
        [](std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
            return std::make_unique<Test1ExecuteMiddleware>(std::move(handle));
        },
        "test1");

    stack->Apply();

    auto request = std::make_unique<RequestMessage>();
    ExecuteContext context;
    auto response = stack->Execute(request, context);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->statusCode, 200);
    ASSERT_NE(response->metrics, nullptr);
    EXPECT_EQ(response->metrics->dnsLookup, std::chrono::microseconds(1500));
    EXPECT_EQ(response->metrics->connect, std::chrono::microseconds(3200));
    EXPECT_EQ(response->metrics->tlsHandshake, std::chrono::microseconds(8500));
    EXPECT_EQ(response->metrics->startTransfer, std::chrono::microseconds(12000));
    EXPECT_EQ(response->metrics->total, std::chrono::microseconds(45000));
    EXPECT_FALSE(response->metrics->connectionReused);
}

TEST(ExecuteStackTest, NullMetricsPropagatesThroughStack) {
    auto httpTransport = std::make_shared<MockHttpTransport>();
    auto fn = [httpTransport]() -> std::unique_ptr<ExecuteMiddleware> {
        return std::make_unique<TransportExecuteMiddleware>(httpTransport);
    };
    auto stack = std::make_unique<ExecuteStack>(fn);
    stack->Apply();

    auto request = std::make_unique<RequestMessage>();
    ExecuteContext context;
    auto response = stack->Execute(request, context);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->metrics, nullptr);
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud