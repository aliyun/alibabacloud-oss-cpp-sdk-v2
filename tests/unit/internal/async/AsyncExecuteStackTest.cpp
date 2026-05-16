#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "src/internal/async/AsyncExecuteStack.h"
#include "src/internal/async/AsyncExecuteMiddleware.h"
#include "src/internal/async/RetryerAsyncMiddleware.h"
#include "src/internal/async/SignerAsyncMiddleware.h"
#include "src/internal/async/ResponseCheckerAsyncMiddleware.h"

#include <condition_variable>
#include <mutex>
#include <sstream>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace internal {

class MockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   RequestContext context,
                   RequestCallback callback) override {
        requestCount++;
        lastUri = request->uri;
        auto result = popResponse();
        callback(std::move(result), std::move(request), std::move(context));
    }

    std::string getName() const override { return "MockAsyncTransport"; }

    void pushResponse(ResponseResult r) {
        responses.emplace_back(std::move(r));
    }

    int requestCount{0};
    std::string lastUri;

  private:
    ResponseResult popResponse() {
        if (!responses.empty()) {
            auto r = std::move(responses.front());
            responses.erase(responses.begin());
            return r;
        }
        auto resp = std::make_unique<ResponseMessage>();
        resp->statusCode = 200;
        return resp;
    }
    std::vector<ResponseResult> responses;
};

struct StackTestHelper {
    std::mutex mtx;
    std::condition_variable cv;
    bool done{false};
    std::shared_ptr<AsyncExecuteState> finalState;

    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return done; });
    }

    void notify(const std::shared_ptr<AsyncExecuteState>& state) {
        std::lock_guard<std::mutex> lock(mtx);
        finalState = state;
        done = true;
        cv.notify_one();
    }
};

class TrackingMiddleware final : public AsyncExecuteMiddleware {
  public:
    TrackingMiddleware(std::unique_ptr<AsyncExecuteMiddleware> next, std::vector<std::string>& log,
                       const std::string& name)
            : next_(std::move(next)), log_(log), name_(name) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) override {
        log_.push_back(name_ + "::request");
        next_->handleRequest(state);
    }

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        log_.push_back(name_ + "::response");
        prev_->handleResponse(state);
    }

  private:
    std::unique_ptr<AsyncExecuteMiddleware> next_;
    std::vector<std::string>& log_;
    std::string name_;
};

class TerminalMiddleware final : public AsyncExecuteMiddleware {
  public:
    explicit TerminalMiddleware(StackTestHelper& helper) : helper_(helper) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>&) override {}
    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        helper_.notify(state);
    }

  private:
    StackTestHelper& helper_;
};

TEST(AsyncExecuteStackTest, BasicExecution) {
    auto transport = std::make_shared<MockAsyncTransport>();
    auto resp = std::make_unique<ResponseMessage>();
    resp->statusCode = 200;
    resp->headers["x-test"] = "value";
    transport->pushResponse(std::move(resp));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://bucket.oss-cn-hangzhou.aliyuncs.com/key";

    stack.executeAsync(state);
    helper.wait();

    EXPECT_EQ(1, transport->requestCount);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ResponseMessage>>(helper.finalState->result));
    auto& response = std::get<std::unique_ptr<ResponseMessage>>(helper.finalState->result);
    EXPECT_EQ(200, response->statusCode);
    EXPECT_EQ("value", response->headers["x-test"]);
}

TEST(AsyncExecuteStackTest, MiddlewareOrdering) {
    auto transport = std::make_shared<MockAsyncTransport>();
    auto resp = std::make_unique<ResponseMessage>();
    resp->statusCode = 200;
    transport->pushResponse(std::move(resp));

    std::vector<std::string> log;
    StackTestHelper helper;

    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);

    // Push Retryer (outermost)
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");

    // Push two tracking middlewares to verify order
    stack.Push(
        [&log](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<TrackingMiddleware>(std::move(next), log, "A");
        }, "A");

    stack.Push(
        [&log](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<TrackingMiddleware>(std::move(next), log, "B");
        }, "B");

    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    stack.executeAsync(state);
    helper.wait();

    // Request flows: Retryer -> A -> B -> Transport
    // Response flows: Transport -> B -> A -> Retryer -> onFinished
    ASSERT_GE(log.size(), 4ULL);
    EXPECT_EQ("A::request", log[0]);
    EXPECT_EQ("B::request", log[1]);
    EXPECT_EQ("B::response", log[2]);
    EXPECT_EQ("A::response", log[3]);
}

TEST(AsyncExecuteStackTest, TransportErrorCodeSetsContextError) {
    auto transport = std::make_shared<MockAsyncTransport>();
    transport->pushResponse(make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    stack.executeAsync(state);
    helper.wait();

    EXPECT_TRUE(helper.finalState->context.errorContext.error);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::CURLE_COULDNT_CONNECT),
              helper.finalState->context.errorContext.error.value());
}

TEST(AsyncExecuteStackTest, TransportResponsePreservesOstreamFactory) {
    auto transport = std::make_shared<MockAsyncTransport>();
    auto resp = std::make_unique<ResponseMessage>();
    resp->statusCode = 200;
    transport->pushResponse(std::move(resp));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Apply();

    bool factoryCalled = false;
    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    state->context.transportContext.ostreamFactory = OStreamFactory{
        [&factoryCalled](int64_t) -> std::shared_ptr<std::ostream> {
            factoryCalled = true;
            return std::make_shared<std::stringstream>();
        },
        false
    };
    stack.executeAsync(state);
    helper.wait();

    // ostreamFactory should still be present on the state after transport completes
    EXPECT_TRUE(helper.finalState->context.transportContext.ostreamFactory.has_value());
    // Call it to verify it's the same factory
    helper.finalState->context.transportContext.ostreamFactory.value()(0);
    EXPECT_TRUE(factoryCalled);
}

TEST(AsyncExecuteStackTest, TransportErrorPreservesOstreamFactory) {
    auto transport = std::make_shared<MockAsyncTransport>();
    transport->pushResponse(make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Apply();

    bool factoryCalled = false;
    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    state->context.transportContext.ostreamFactory = OStreamFactory{
        [&factoryCalled](int64_t) -> std::shared_ptr<std::ostream> {
            factoryCalled = true;
            return std::make_shared<std::stringstream>();
        },
        false
    };
    stack.executeAsync(state);
    helper.wait();

    EXPECT_TRUE(helper.finalState->context.transportContext.ostreamFactory.has_value());
    helper.finalState->context.transportContext.ostreamFactory.value()(0);
    EXPECT_TRUE(factoryCalled);
}

TEST(AsyncExecuteStackTest, TransportErrorContextFields) {
    auto transport = std::make_shared<MockAsyncTransport>();

    // Return error_code with errorCode/errorMessage in context
    transport->pushResponse(make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    // Use a custom transport that sets errorCode/errorMessage
    class ErrorTransport : public AsyncHttpTransport {
      public:
        void sendAsync(std::unique_ptr<RequestMessage> request,
                       RequestContext context,
                       RequestCallback callback) override {
            context.errorCode = "CURLcode 7";
            context.errorMessage = "Could not connect";
            callback(make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT),
                     std::move(request), std::move(context));
        }
        std::string getName() const override { return "ErrorTransport"; }
    };

    auto errorTransport = std::make_shared<ErrorTransport>();
    AsyncExecuteStack stack(errorTransport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    stack.executeAsync(state);
    helper.wait();

    EXPECT_TRUE(helper.finalState->context.errorContext.error);
    // TransportAsyncMiddleware moves errorCode/errorMessage into errorFields
    auto& fields = helper.finalState->context.errorContext.errorFields;
    EXPECT_EQ("CURLcode 7", fields["Code"]);
    EXPECT_EQ("Could not connect", fields["Message"]);
}

TEST(AsyncExecuteStackTest, ResponseCheckerInvokesCallbacks) {
    auto transport = std::make_shared<MockAsyncTransport>();
    auto resp = std::make_unique<ResponseMessage>();
    resp->statusCode = 404;
    resp->body = std::make_shared<std::stringstream>(
        "<?xml version=\"1.0\"?><Error><Code>NoSuchKey</Code><Message>not found</Message></Error>");
    transport->pushResponse(std::move(resp));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<ResponseCheckerAsyncMiddleware>(std::move(next));
        }, "ResponseChecker");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";

    // Add onServiceError-like callback
    bool callbackInvoked = false;
    state->context.onResponseMessage.emplace_back(
        [&callbackInvoked](std::unique_ptr<ResponseMessage>& response, ExecuteContext& context) -> bool {
            callbackInvoked = true;
            if (response->statusCode / 100 != 2) {
                context.errorContext.error = make_error_code(
                    static_cast<SdkErrorCode>(response->statusCode));
                return false;
            }
            return true;
        });

    stack.executeAsync(state);
    helper.wait();

    EXPECT_TRUE(callbackInvoked);
    EXPECT_TRUE(helper.finalState->context.errorContext.error);
    EXPECT_EQ(404, helper.finalState->context.errorContext.error.value());
}

TEST(AsyncExecuteStackTest, ResponseCheckerSkippedOnError) {
    auto transport = std::make_shared<MockAsyncTransport>();
    transport->pushResponse(make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT));

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<ResponseCheckerAsyncMiddleware>(std::move(next));
        }, "ResponseChecker");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";

    bool callbackInvoked = false;
    state->context.onResponseMessage.emplace_back(
        [&callbackInvoked](std::unique_ptr<ResponseMessage>&, ExecuteContext&) -> bool {
            callbackInvoked = true;
            return true;
        });

    stack.executeAsync(state);
    helper.wait();

    // ResponseChecker should NOT invoke callbacks when error is already set
    EXPECT_FALSE(callbackInvoked);
    EXPECT_TRUE(helper.finalState->context.errorContext.error);
}

TEST(AsyncExecuteStackTest, SignerErrorShortCircuits) {
    auto transport = std::make_shared<MockAsyncTransport>();

    StackTestHelper helper;
    auto onFinished = [&helper](const std::shared_ptr<AsyncExecuteState>& s) {
        helper.notify(s);
    };

    AsyncExecuteStack stack(transport, onFinished);
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<RetryerAsyncMiddleware>(
                std::move(next), std::make_shared<NopRetryer>());
        }, "Retryer");
    // Signer with null provider triggers error
    stack.Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) {
            return std::make_unique<SignerAsyncMiddleware>(std::move(next), nullptr, nullptr);
        }, "Signer");
    stack.Apply();

    auto state = std::make_shared<AsyncExecuteState>();
    state->request = std::make_unique<RequestMessage>();
    state->request->uri = "https://example.com";
    stack.executeAsync(state);
    helper.wait();

    // Transport should NOT have been called
    EXPECT_EQ(0, transport->requestCount);
    EXPECT_TRUE(helper.finalState->context.errorContext.error);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::CREDENTIALS_PROVIDER_NULL),
              helper.finalState->context.errorContext.error.value());
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
