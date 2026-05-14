#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace alibabacloud::oss2 {

namespace {

class MockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   RequestContext context,
                   RequestCallback callback) override {
        ResponseResult responseResult = std::make_error_code(std::errc::result_out_of_range);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto req = std::make_unique<RequestMessage>(*request);
            lastRequest = req.get();
            requests.emplace_back(std::move(req));

            if (!responses.empty()) {
                responseResult = std::move(responses.front());
                responses.erase(responses.begin());
            }
        }
        callback(std::move(responseResult), std::move(request), std::move(context));
    }

    std::string getName() const override {
        return "MockAsyncTransport";
    }

    std::vector<std::unique_ptr<ResponseMessage>> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    RequestMessage* lastRequest = nullptr;
    std::mutex mutex_;

    void Clear() {
        responses.clear();
        requests.clear();
        lastRequest = nullptr;
    }
};

std::future<OperationResult> invokeAsync(OSSAsyncClient& client, const OperationInput& input,
                                         const OperationOptions* options = nullptr) {
    auto promise = std::make_shared<std::promise<OperationResult>>();
    client.invokeOperationAsync(input, [promise](OperationResult result) {
        promise->set_value(std::move(result));
    }, options);
    return promise->get_future();
}

} // namespace

TEST(OSSAsyncClientTest, DefaultCtor) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);
}

TEST(OSSAsyncClientTest, InvokeOperation_Callback_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK",
                    {{"x-oss-request-id", "req-123"}},
                    nullptr}));

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    OperationResult captured;

    OperationInput input;
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "test-bucket";
    input.key = "test-key";

    client.invokeOperationAsync(input,
                                [&](OperationResult result) {
                                    std::lock_guard<std::mutex> lock(mtx);
                                    captured = std::move(result);
                                    done = true;
                                    cv.notify_one();
                                });

    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return done; });

    ASSERT_TRUE(std::holds_alternative<OperationOutput>(captured));
    auto& output = std::get<OperationOutput>(captured);
    EXPECT_EQ(output.statusCode, 200);
    EXPECT_EQ(output.headers.at("x-oss-request-id"), "req-123");

    ASSERT_NE(mockTransport->lastRequest, nullptr);
    EXPECT_TRUE(mockTransport->lastRequest->uri.find("test-bucket") != std::string::npos);
}

TEST(OSSAsyncClientTest, InvokeOperation_Future_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK",
                    {{"x-oss-request-id", "req-456"}},
                    nullptr}));

    OperationInput input;
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "test-bucket";
    input.key = "test-key";
    input.body = RequestBody::FromString("hello world");

    auto future = invokeAsync(client, input);
    auto result = future.get();

    ASSERT_TRUE(std::holds_alternative<OperationOutput>(result));
    auto& output = std::get<OperationOutput>(result);
    EXPECT_EQ(output.statusCode, 200);
}

TEST(OSSAsyncClientTest, InvokeOperation_TransportError) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    OperationInput input;
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "test-bucket";
    input.key = "test-key";

    auto future = invokeAsync(client, input);
    auto result = future.get();

    ASSERT_TRUE(std::holds_alternative<OperationError>(result));
}

TEST(OSSAsyncClientTest, InvokeOperation_InvalidInput) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    OperationInput input;

    auto future = invokeAsync(client, input);
    auto result = future.get();

    ASSERT_TRUE(std::holds_alternative<OperationError>(result));
}

TEST(OSSAsyncClientTest, InvokeOperation_MultipleAsync) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    const int numRequests = 5;
    for (int i = 0; i < numRequests; i++) {
        mockTransport->responses.emplace_back(
                std::make_unique<ResponseMessage>(ResponseMessage{
                        200, "OK", {}, nullptr}));
    }

    std::vector<std::future<OperationResult>> futures;
    for (int i = 0; i < numRequests; i++) {
        OperationInput input;
        input.opName = "GetObject";
        input.method = "GET";
        input.bucket = "test-bucket";
        input.key = "key-" + std::to_string(i);
        futures.push_back(invokeAsync(client, input));
    }

    for (auto& f : futures) {
        auto result = f.get();
        ASSERT_TRUE(std::holds_alternative<OperationOutput>(result));
        EXPECT_EQ(std::get<OperationOutput>(result).statusCode, 200);
    }
}

TEST(OSSAsyncClientTest, InvokeOperation_WithCredentials) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK", {}, nullptr}));

    OperationInput input;
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "test-bucket";
    input.key = "test-key";

    auto future = invokeAsync(client, input);
    auto result = future.get();

    ASSERT_TRUE(std::holds_alternative<OperationOutput>(result));
    ASSERT_NE(mockTransport->lastRequest, nullptr);
    EXPECT_TRUE(mockTransport->lastRequest->headers.find("Authorization") != mockTransport->lastRequest->headers.end());
}

} // namespace alibabacloud::oss2
