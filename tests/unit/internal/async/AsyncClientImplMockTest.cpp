#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/signer/Signer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "src/internal/async/AsyncClientImpl.h"
#include "src/internal/Defaults.h"
#include "src/utils/Utils.h"

#include <condition_variable>
#include <mutex>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {

static ClientOptionsFns asyncDefaultClientFns;

class MockAsyncTransportImpl : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions&,
                   RequestCallback callback) override {
        saveRequest(request);
        auto result = popResponse();
        callback(std::move(result), std::move(request));
    }

    std::string getName() const override {
        return "MockAsyncTransport";
    }

    std::vector<std::unique_ptr<RequestMessage>> requests;
    std::vector<ResponseResult> responses;
    RequestMessage* lastRequest = nullptr;

    void Clear() {
        requests.clear();
        responses.clear();
        lastRequest = nullptr;
    }

  private:
    void saveRequest(std::unique_ptr<RequestMessage>& request) {
        auto req = std::make_unique<RequestMessage>(*request);
        lastRequest = req.get();
        requests.emplace_back(std::move(req));

        if (lastRequest->body != nullptr) {
            auto src = lastRequest->body->spanSource();
            src->readToEnd();
        }
    }

    ResponseResult popResponse() {
        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return TransportError{std::make_error_code(std::errc::result_out_of_range)};
    }
};

struct AsyncTestHelper {
    std::mutex mtx;
    std::condition_variable cv;
    bool done{false};
    OperationResult result;

    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return done; });
    }

    OperationCallback callback() {
        return [this](OperationResult r) {
            std::lock_guard<std::mutex> lock(mtx);
            result = std::move(r);
            done = true;
            cv.notify_one();
        };
    }
};


TEST(AsyncClientImplMockTest, InvokeOperationSuccess) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&helper.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
}


TEST(AsyncClientImplMockTest, InvokeOperationFail) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    std::string errorXml =
            R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
    <OSSAccessKeyId>ak</OSSAccessKeyId>
    <EC>0002-00000902</EC>
    <RecommendDoc>https://api.aliyun.com/troubleshoot?q=0002-00000902</RecommendDoc>
</Error>
)";

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("PutBucketAcl", error->getOpName());
    EXPECT_EQ("PUT", error->getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?acl", error->getRequestTarget());
    EXPECT_EQ("id-1234", error->getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error->getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error->getMessage());
    EXPECT_EQ("0002-00000902", error->getEC());
}


TEST(AsyncClientImplMockTest, verifyExecuteArgsInvalidEndpoint) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();
    auto config = ClientConfiguration::loadDefault();
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(0ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("endpoint or region is invalid", error->getMessage());
}


TEST(AsyncClientImplMockTest, verifyExecuteArgsInvalidMethod) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "invalid-method";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(0ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("input.method is empty or invalid, got invalid-method.", error->getMessage());
}


TEST(AsyncClientImplMockTest, verifyExecuteArgsInvalidBucketName) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "GET";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket#123";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(0ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("input.bucket is invalid, got bucket#123.", error->getMessage());
}


TEST(AsyncClientImplMockTest, configRetryMaxAttemptsFromClientOptions) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-4"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(3ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-3", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configRetryMaxAttemptsCustom) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    config.retryMaxAttempts = 4;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-4"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(4ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-4", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configRetryMaxAttemptsFromOperationOptions) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-4"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto opts = OperationOptions{2};

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback(), &opts);
    helper.wait();

    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-2", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configRetryMaxAttemptsNopRetryer) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    config.retryer = std::make_shared<NopRetryer>();

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configNoRetryError) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configSeekableStream) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("hello world");

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(3ULL, mockHandler->requests.size());
    EXPECT_EQ(11ULL, mockHandler->lastRequest->body->length().value());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-3", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configNoSeekableStream) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = std::make_shared<StreamContent>(std::make_shared<std::stringstream>("hello world"), false);

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    EXPECT_EQ(false, mockHandler->lastRequest->body->length().has_value());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(AsyncClientImplMockTest, configSignerV4) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "123/+%/abc.txt";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&helper.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);

    auto date = utils::FormatUnixTime(std::time(nullptr), "%Y%m%d");

    EXPECT_EQ("PUT", mockHandler->lastRequest->method);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/123/%2B%25/abc.txt", mockHandler->lastRequest->uri);
    auto authPat = "OSS4-HMAC-SHA256 Credential=ak/" + date + "/cn-hangzhou/oss/aliyun_v4_request,Signature=";
    EXPECT_EQ(authPat, mockHandler->lastRequest->headers.at("Authorization").substr(0, authPat.size()));
}


TEST(AsyncClientImplMockTest, sendAnonymousRequest) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "123/+%/abc.txt";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&helper.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);

    EXPECT_EQ("PUT", mockHandler->lastRequest->method);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/123/%2B%25/abc.txt", mockHandler->lastRequest->uri);
    EXPECT_EQ(mockHandler->lastRequest->headers.end(), mockHandler->lastRequest->headers.find("Authorization"));
}


TEST(AsyncClientImplMockTest, returnsEmptyCredentials) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("", "");
    config.asyncHttpTransport = mockHandler;
    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(0ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("PutBucketAcl", error->getOpName());
    EXPECT_EQ("PUT", error->getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?acl", error->getRequestTarget());
    EXPECT_EQ("CredentialsError", error->getCode());
    EXPECT_EQ("Credentials is null or empty.", error->getMessage());
}


TEST(AsyncClientImplMockTest, transportErrorRetryable) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(TransportError{make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT)});
    mockHandler->responses.emplace_back(TransportError{make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT)});
    mockHandler->responses.emplace_back(TransportError{make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT)});

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(3ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
}


TEST(AsyncClientImplMockTest, transportErrorNonRetryable) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(TransportError{std::make_error_code(std::errc::connection_refused)});

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
}


TEST(AsyncClientImplMockTest, useVirtualHostAddressingMode) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&helper.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/", mockHandler->lastRequest->uri);

    // bucket
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "ListObjects";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.bucket = "bucket";

    AsyncTestHelper helper2;
    client.ExecuteAsync(input, helper2.callback());
    helper2.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&helper2.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/", mockHandler->lastRequest->uri);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "key-prefix/123.txt";

    AsyncTestHelper helper3;
    client.ExecuteAsync(input, helper3.callback());
    helper3.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&helper3.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/key-prefix/123.txt", mockHandler->lastRequest->uri);
}


TEST(AsyncClientImplMockTest, usePathAddressingMode) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;
    config.usePathStyle = true;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";
    input.key = "my-key+123";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&helper.result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/my-bucket/my-key%2B123?key=value", mockHandler->lastRequest->uri);
}


TEST(AsyncClientImplMockTest, returnsServiceExceptionNormal) {
    auto mockHandler = std::make_shared<MockAsyncTransportImpl>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockHandler;

    auto client = AsyncClientImpl(config, asyncDefaultClientFns);

    std::string errorXml = R"(<Error>
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </Error>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    AsyncTestHelper helper;
    client.ExecuteAsync(input, helper.callback());
    helper.wait();

    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&helper.result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("5C3D9175B6FC201293AD****", error->getRequestId());
    EXPECT_EQ("NoSuchBucket", error->getCode());
    EXPECT_EQ("The specified bucket does not exist.", error->getMessage());
    EXPECT_EQ("0015-00000101", error->getEC());
    EXPECT_EQ(errorXml, error->getSnapshot());
}


} // namespace internal
} // namespace oss2
} // namespace alibabacloud
