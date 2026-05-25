#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/signer/Signer.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/internal/Defaults.h"
#include "src/utils/Utils.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {

const static std::string ErrorXml =
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

static ClientOptionsFns defaultClientFns;

class MockTransport : public HttpTransport {
  public:
    MockTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        saveRequest(request);
        return popResponse();
    }
    std::string getName() const override {
        return "MockTransport";
    }

  public:
    std::vector<std::unique_ptr<RequestMessage>> requests;
    std::vector<std::unique_ptr<ResponseMessage>> responses;
    RequestMessage* lastRequest = nullptr;
    std::vector<std::string> requestDates;

    void Clear() {
        requests.clear();
        responses.clear();
        requestDates.clear();
        lastRequest = nullptr;
    }

  private:
    void saveRequest(std::unique_ptr<RequestMessage>& request) {
        auto req = std::make_unique<RequestMessage>(*request);
        lastRequest = req.get();
        requests.emplace_back(std::move(req));

        if (request->headers.find("Date") != request->headers.end()) {
            requestDates.emplace_back(request->headers.at("Date"));
        }

        // read data
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


TEST(ClientImplMockTest, InvokeOperationSuccess) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
}


TEST(ClientImplMockTest, InvokeOperationFail) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(ErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("PutBucketAcl", error->getOpName());
    EXPECT_EQ("PUT", error->getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?acl", error->getRequestTarget());
    EXPECT_EQ("id-1234", error->getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error->getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error->getMessage());
    EXPECT_EQ("0002-00000902", error->getEC());
    EXPECT_EQ(7, error->getErrorFields().size());
    EXPECT_EQ("ak", error->getErrorFields().at("OSSAccessKeyId"));
    EXPECT_EQ(1, error->getHeaders().size());
    EXPECT_EQ("id-12345", error->getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error->getSnapshot().substr(0, 7));
}

TEST(ClientImplMockTest, verifyExecuteArgsInvalidEndpoint) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    // config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getOpName());
    EXPECT_EQ("", error->getMethod());
    EXPECT_EQ("", error->getRequestTarget());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("endpoint or region is invalid", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ(2, error->getErrorFields().size());
    EXPECT_EQ(0, error->getHeaders().size());
    EXPECT_EQ("", error->getSnapshot());
}

TEST(ClientImplMockTest, verifyExecuteArgsInvalidMethod) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "invalid-method";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getOpName());
    EXPECT_EQ("", error->getMethod());
    EXPECT_EQ("", error->getRequestTarget());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("input.method is empty or invalid, got invalid-method.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ(2, error->getErrorFields().size());
    EXPECT_EQ(0, error->getHeaders().size());
    EXPECT_EQ("", error->getSnapshot());
}


TEST(ClientImplMockTest, verifyExecuteArgsInvalidBucketName) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "GET";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket#123";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getOpName());
    EXPECT_EQ("", error->getMethod());
    EXPECT_EQ("", error->getRequestTarget());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("input.bucket is invalid, got bucket#123.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ(2, error->getErrorFields().size());
    EXPECT_EQ(0, error->getHeaders().size());
    EXPECT_EQ("", error->getSnapshot());
}


TEST(ClientImplMockTest, verifyExecuteArgsInvalidObjectName) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    // input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";
    input.key = "\\";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getOpName());
    EXPECT_EQ("", error->getMethod());
    EXPECT_EQ("", error->getRequestTarget());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("input.key is invalid, got \\.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ(2, error->getErrorFields().size());
    EXPECT_EQ(0, error->getHeaders().size());
    EXPECT_EQ("", error->getSnapshot());
}


TEST(ClientImplMockTest, configRetryMaxAttemptsFromClientOptions) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    // default max retry attempts is 3
    auto client = ClientImpl(config, defaultClientFns);

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

    auto result = client.Execute(input);
    EXPECT_EQ(3, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-3", error->getRequestId());

    //  max retry attempts is 4
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.retryMaxAttempts = 4;

    client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-4"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    result = client.Execute(input);
    EXPECT_EQ(4, mockHandler->requests.size());
    error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-4", error->getRequestId());
}


TEST(ClientImplMockTest, configRetryMaxAttemptsFromOperationOptions) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

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

    // default max retry attempts is 3, set to 2 from OperationOptions
    auto opts = OperationOptions{2};

    auto result = client.Execute(input, &opts);
    EXPECT_EQ(2, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-2", error->getRequestId());
}


TEST(ClientImplMockTest, configRetryMaxAttemptsNopRetryer) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.retryer = std::make_shared<NopRetryer>();

    auto client = ClientImpl(config, defaultClientFns);

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

    auto result = client.Execute(input);
    EXPECT_EQ(1, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(ClientImplMockTest, configNoRetryError) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
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

    // default max retry attempts is 3, but meets no retryable error
    auto result = client.Execute(input);
    EXPECT_EQ(1, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(ClientImplMockTest, configSeekableStream) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    // default max retry attempts is 3
    auto client = ClientImpl(config, defaultClientFns);

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
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";
    input.bucket = "key";
    input.body = RequestBody::FromString("hello world");

    auto result = client.Execute(input);
    EXPECT_EQ(3, mockHandler->requests.size());
    EXPECT_EQ(11, mockHandler->lastRequest->body->length().value());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-3", error->getRequestId());
}


TEST(ClientImplMockTest, configNoSeekableStream) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    // default max retry attempts is 3
    auto client = ClientImpl(config, defaultClientFns);

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
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";
    input.bucket = "key";
    input.body = std::make_shared<StreamContent>(std::make_shared<std::stringstream>("hello world"), false);

    auto result = client.Execute(input);
    EXPECT_EQ(1, mockHandler->requests.size());
    EXPECT_EQ(false, mockHandler->lastRequest->body->length().has_value());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-1", error->getRequestId());
}


TEST(ClientImplMockTest, checkBackoffSleepTime) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.retryer =
            std::make_shared<StandardRetryer>(3, std::make_unique<FixedDelayBackoff>(std::chrono::milliseconds(1000)));

    // default max retry attempts is 3
    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-3"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-4"}}, std::make_shared<std::stringstream>("")}));

    auto start = std::chrono::system_clock::now();

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(3, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(500, error->getStatusCode());
    EXPECT_EQ("id-3", error->getRequestId());

    auto diff = std::chrono::system_clock::now() - start;
    EXPECT_GT(diff, std::chrono::milliseconds(2000));
    EXPECT_LT(diff, std::chrono::milliseconds(3000));
}

TEST(ClientImplMockTest, configSignerV4) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "123/+%/abc.txt";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    auto date = utils::FormatUnixTime(std::time(nullptr), "%Y%m%d");

    EXPECT_EQ(1, mockHandler->requests.size());
    EXPECT_EQ("PUT", mockHandler->lastRequest->method);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/123/%2B%25/abc.txt", mockHandler->lastRequest->uri);
    auto authPat = "OSS4-HMAC-SHA256 Credential=ak/" + date + "/cn-hangzhou/oss/aliyun_v4_request,Signature=";
    EXPECT_EQ(authPat, mockHandler->lastRequest->headers.at("Authorization").substr(0, authPat.size()));
}


TEST(ClientImplMockTest, configSignerV1) {}

TEST(ClientImplMockTest, sendAnonymousRequest) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"x-oss-object-acl", "private"}, {"Content-Type", "application/xml"}};
    input.bucket = "bucket";
    input.key = "123/+%/abc.txt";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    auto date = utils::FormatUnixTime(std::time(nullptr), "%Y%m%d");

    EXPECT_EQ(1, mockHandler->requests.size());
    EXPECT_EQ("PUT", mockHandler->lastRequest->method);
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/123/%2B%25/abc.txt", mockHandler->lastRequest->uri);
    EXPECT_EQ(mockHandler->lastRequest->headers.end(), mockHandler->lastRequest->headers.find("Authorization"));
}

TEST(ClientImplMockTest, returnsEmptyCredentials) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("", "");
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("PutBucketAcl", error->getOpName());
    EXPECT_EQ("PUT", error->getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?acl", error->getRequestTarget());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("CredentialsError", error->getCode());
    EXPECT_EQ("Credentials is null or empty.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ(2, error->getErrorFields().size());
    EXPECT_EQ(0, error->getHeaders().size());
    EXPECT_EQ("", error->getSnapshot());
}


TEST(ClientImplMockTest, returnsNullCredentials) {}

TEST(ClientImplMockTest, returnsCredentialsWithError) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<CredentialsProviderFunc>([]() {
        return Credentials::withError("STS token refresh failed: connection timeout");
    });
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ("CredentialsError", error->getCode());
    EXPECT_EQ("STS token refresh failed: connection timeout", error->getMessage());
}


TEST(ClientImplMockTest, returnsRetryableCredentialsError) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<CredentialsProviderFunc>([]() {
        return Credentials::withRetryableError("STS service unavailable");
    });
    config.httpTransport = mockHandler;
    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(0, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ("CredentialsError", error->getCode());
    EXPECT_EQ("STS service unavailable", error->getMessage());
    EXPECT_EQ(error->getErrorCode(), make_error_condition(ErrorCondition::Retryable));
}

TEST(ClientImplMockTest, useVirtualHostAddressingMode) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

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

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

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

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/key-prefix/123.txt", mockHandler->lastRequest->uri);
}

TEST(ClientImplMockTest, usePathAddressingMode) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.usePathStyle = true;

    auto client = ClientImpl(config, defaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/", mockHandler->lastRequest->uri);


    // bucket
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "ListObjects";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/my-bucket/?key=value", mockHandler->lastRequest->uri);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";
    input.key = "my-key+123";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/my-bucket/my-key%2B123?key=value", mockHandler->lastRequest->uri);
}

TEST(ClientImplMockTest, useCNameAddressingMode) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.useCName = true;
    config.endpoint = "http://www.cname.com";
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://www.cname.com/?key=value", mockHandler->lastRequest->uri);


    // bucket
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "ListObjects";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://www.cname.com/?key=value", mockHandler->lastRequest->uri);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";
    input.key = "my-key+123";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://www.cname.com/my-key%2B123?key=value", mockHandler->lastRequest->uri);
}

TEST(ClientImplMockTest, useIpEndpoint) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.endpoint = "http://192.168.1.1:8080";

    auto client = ClientImpl(config, defaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/?key=value", mockHandler->lastRequest->uri);


    // bucket
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "ListObjects";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/my-bucket/?key=value", mockHandler->lastRequest->uri);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";
    input.key = "my-key+123";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/my-bucket/my-key%2B123?key=value", mockHandler->lastRequest->uri);
}

TEST(ClientImplMockTest, useIpEndpointWithQuery) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.endpoint = "http://192.168.1.1:8080/path/?key=123#segment";

    auto client = ClientImpl(config, defaultClientFns);

    // no bucket & key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "ListBuckets";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/?key=value", mockHandler->lastRequest->uri);


    // bucket
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "ListObjects";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/my-bucket/?key=value", mockHandler->lastRequest->uri);

    // bucket and key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.headers = {{"Content-Type", "application/xml"}};
    input.parameters = {{"key", "value"}};
    input.bucket = "my-bucket";
    input.key = "my-key+123";

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    EXPECT_EQ("http://192.168.1.1:8080/my-bucket/my-key%2B123?key=value", mockHandler->lastRequest->uri);
}

TEST(ClientImplMockTest, returnsServiceExceptionNormal) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(<Error> 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </Error>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("5C3D9175B6FC201293AD****", error->getRequestId());
    EXPECT_EQ("NoSuchBucket", error->getCode());
    EXPECT_EQ("The specified bucket does not exist.", error->getMessage());
    EXPECT_EQ("0015-00000101", error->getEC());
    EXPECT_EQ(errorXml, error->getSnapshot());
}

TEST(ClientImplMockTest, returnsServiceExceptionInHeader) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(<Error> 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </Error>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404,
                                                              "Not Found",
                                                              {{"x-oss-request-id", "5C3D9175B6FC201293AD****"},
                                                               {"Content-Type", "application/xml"},
                                                               {"Date", "Fri, 24 Feb 2017 03:15:40 GMT"},
                                                               {"x-oss-err", utils::Base64Encode(errorXml)}},
                                                              std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("5C3D9175B6FC201293AD****", error->getRequestId());
    EXPECT_EQ("NoSuchBucket", error->getCode());
    EXPECT_EQ("The specified bucket does not exist.", error->getMessage());
    EXPECT_EQ("0015-00000101", error->getEC());
    EXPECT_EQ(errorXml, error->getSnapshot());
}

TEST(ClientImplMockTest, returnsServiceExceptionEmptyBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(<Error> 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </Error>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-12345", error->getRequestId());
    EXPECT_EQ("BadErrorResponse", error->getCode());
    EXPECT_EQ("Empty body", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("", error->getSnapshot());
}

TEST(ClientImplMockTest, returnsServiceExceptionNotErrorFormat) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(<NotError> 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </NotError>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-12345", error->getRequestId());
    EXPECT_EQ("ParseXMLError", error->getCode());
    std::string pat = "Xml format invalid, root node name is not Error. the content";
    EXPECT_EQ(pat, error->getMessage().substr(0, pat.size()));
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("<NotError>", error->getSnapshot().substr(0, 10));
}

TEST(ClientImplMockTest, returnsServiceExceptionNotXmlFormat) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(NotError 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <EC>0015-00000101</EC>
    </NotError>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-12345", error->getRequestId());
    EXPECT_EQ("ParseXMLError", error->getCode());
    std::string pat = "Xml format invalid, root node name is not Error. the content";
    EXPECT_EQ(pat, error->getMessage().substr(0, pat.size()));
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("NotError \n", error->getSnapshot().substr(0, 10));
}

TEST(ClientImplMockTest, returnsServiceExceptionComplexErrorFormat) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string errorXml = R"(<Error> 
        <Code>NoSuchBucket</Code>
        <Message>The specified bucket does not exist.</Message>
        <RequestId>5C3D9175B6FC201293AD****</RequestId>
        <HostId>test.oss-cn-hangzhou.aliyuncs.com</HostId>
        <BucketName>test</BucketName>
        <InnerError>
            <Field1>filed-1-value</Field1>
            <Field2>filed-2-value</Field2>
        </InnerError>
        <EC>0015-00000101</EC>
    </Error>
    )";

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("5C3D9175B6FC201293AD****", error->getRequestId());
    EXPECT_EQ("NoSuchBucket", error->getCode());
    EXPECT_EQ("The specified bucket does not exist.", error->getMessage());
    EXPECT_EQ("0015-00000101", error->getEC());
    EXPECT_EQ(errorXml, error->getSnapshot());
    EXPECT_EQ(error->getErrorFields().end(), error->getErrorFields().find("InnerError"));
}

TEST(ClientImplMockTest, returnsServiceExceptionNullBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // error in xml body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{404, "Not Found", {{"x-oss-request-id", "id-12345"}}, nullptr}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.headers = {{"x-oss-acl", "private"}, {"Content-Type", "application/xml"}};
    input.parameters = {{"acl", ""}};
    input.bucket = "bucket";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);

    EXPECT_NE(nullptr, error);
    EXPECT_EQ(404, error->getStatusCode());
    EXPECT_EQ("id-12345", error->getRequestId());
    EXPECT_EQ("BadErrorResponse", error->getCode());
    EXPECT_EQ("Empty body", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("", error->getSnapshot());
}

TEST(ClientImplMockTest, testUploadObserverNormal_useCRCObserver) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // not set observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    // empty observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    OperationInnerOptions innerOpts{};
    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);

    // set crc observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    auto crc64Observer = std::make_shared<CRC64Observer>();
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ("2004446556369382352", std::to_string(crc64Observer->crc()));


    // reset crc64Observer
    innerOpts = {};
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    crc64Observer->reset();
    EXPECT_EQ("0", std::to_string(crc64Observer->crc()));
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ("2004446556369382352", std::to_string(crc64Observer->crc()));
}

TEST(ClientImplMockTest, testUploadObserver_useMultiObserver) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    std::string data = "Hello, OSS!";

    auto innerOpts = OperationInnerOptions{};

    // set crc observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString(data);

    std::size_t _transferred = 0;
    std::int64_t _total = 0;
    std::size_t _increment = 0;
    ProgressCallback progressCallback{[&_transferred, &_total, &_increment](std::size_t increment,
                                                                            std::size_t transferred, std::int64_t total,
                                                                            std::uintptr_t userdata) -> void {
        // std::cout << "Progress:" << increment << "," << transferred << "," << total << std::endl;
        _increment += increment;
        _transferred = transferred;
        _total = total;
    }};

    innerOpts.uploadObserver.emplace_back(std::make_shared<ProgressObserver>(progressCallback, data.size()));

    auto crc64Observer = std::make_shared<CRC64Observer>();
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    auto result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ("2004446556369382352", std::to_string(crc64Observer->crc()));
    EXPECT_EQ(data.size(), _increment);
    EXPECT_EQ(data.size(), _transferred);
    EXPECT_EQ(data.size(), _total);
}

TEST(ClientImplMockTest, testUploadObserverRetryable_useCRCObserver) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    OperationInput input;
    OperationInnerOptions innerOpts;
    OperationResult result;

    // reset crc64Observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    innerOpts = {};
    auto crc64Observer = std::make_shared<CRC64Observer>();
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ("2004446556369382352", std::to_string(crc64Observer->crc()));
}


TEST(ClientImplMockTest, testUploadObserverRetryable_useProgObserver) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    OperationInput input;
    OperationInnerOptions innerOpts;
    OperationResult result;
    std::string data = "Hello, OSS!";

    // reset crc64Observer
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            500, "Internal Server Error", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    innerOpts = {};
    std::size_t _transferred = 0;
    std::int64_t _total = 0;
    std::size_t _increment = 0;
    ProgressCallback progressCallback{[&_transferred, &_total, &_increment](std::size_t increment,
                                                                            std::size_t transferred, std::int64_t total,
                                                                            std::uintptr_t userdata) -> void {
        // std::cout << "Progress:" << increment << "," << transferred << "," << total << std::endl;
        _increment += increment;
        _transferred = transferred;
        _total = total;
    }};

    innerOpts.uploadObserver.emplace_back(std::make_shared<ProgressObserver>(progressCallback, data.size()));

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(1ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ(data.size(), _increment);
    EXPECT_EQ(data.size(), _transferred);
    EXPECT_EQ(data.size(), _total);
}



TEST(ClientImplMockTest, testUploadDataAndCheckResponseCrc) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    OperationInput input;
    OperationInnerOptions innerOpts;
    OperationResult result;

    mockHandler->Clear();

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-1234"}, {"x-oss-hash-crc64ecma", "123"}},
                            std::make_shared<std::stringstream>("")}));

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-1234"}, {"x-oss-hash-crc64ecma", "2004446556369382352"}},
                            std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    innerOpts = {};
    auto crc64Observer = std::make_shared<CRC64Observer>();
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    innerOpts.onResponseMessage.emplace_back(CRC64ResponseChecker{crc64Observer});

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
    EXPECT_EQ(2ULL, output->headers.size());
    EXPECT_NE(nullptr, output->body);
    EXPECT_EQ("2004446556369382352", std::to_string(crc64Observer->crc()));
}

TEST(ClientImplMockTest, testUploadDataAndCheckResponseCrc_throwInconsistentException) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    OperationInput input;
    OperationInnerOptions innerOpts;
    OperationResult result;

    mockHandler->Clear();

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", "1"}},
                            std::make_shared<std::stringstream>("")}));

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-2"}, {"x-oss-hash-crc64ecma", "2"}},
                            std::make_shared<std::stringstream>("")}));

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-3"}, {"x-oss-hash-crc64ecma", "3"}},
                            std::make_shared<std::stringstream>("")}));

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"x-oss-request-id", "id-4"}, {"x-oss-hash-crc64ecma", "4"}},
                            std::make_shared<std::stringstream>("")}));

    input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.headers = {{"Content-Type", "text/plain"}};
    input.bucket = "bucket";
    input.key = "key";
    input.body = RequestBody::FromString("Hello, OSS!");

    innerOpts = {};
    auto crc64Observer = std::make_shared<CRC64Observer>();
    innerOpts.uploadObserver.emplace_back(crc64Observer);

    innerOpts.onResponseMessage.emplace_back(CRC64ResponseChecker{crc64Observer});

    result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_EQ(3ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(200, error->getStatusCode());
    EXPECT_EQ("CRCInconsistent", error->getCode());
    EXPECT_EQ("crc is inconsistent, client crc:2004446556369382352, server crc:3", error->getMessage());
}


TEST(ClientImplMockTest, presignInnerV4) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto datetime = utils::FormatUnixTime(expiration, "%Y%m%dT%H%M%SZ");
    auto date = datetime.substr(0, 8);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(0LL, output->signedHeaders.size());
    EXPECT_NE(std::string::npos, output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?"));
    EXPECT_NE(std::string::npos,
              output->url.find("x-oss-credential=ak%2F" + date + "%2Fcn-hangzhou%2Foss%2Faliyun_v4_request"));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-date=" + date));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-signature-version=OSS4-HMAC-SHA256"));
    EXPECT_TRUE(output->url.find("x-oss-expires=3599") != std::string::npos ||
                output->url.find("x-oss-expires=3600") != std::string::npos);
}

TEST(ClientImplMockTest, presignInnerV4_defaultSignedHeader) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto datetime = utils::FormatUnixTime(expiration, "%Y%m%dT%H%M%SZ");
    auto date = datetime.substr(0, 8);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(3LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos,
              output->url.find("x-oss-credential=ak%2F" + date + "%2Fcn-hangzhou%2Foss%2Faliyun_v4_request"));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-date=" + date));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-signature-version=OSS4-HMAC-SHA256"));
    EXPECT_TRUE(output->url.find("x-oss-expires=3599") != std::string::npos ||
                output->url.find("x-oss-expires=3600") != std::string::npos);

    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));
}


TEST(ClientImplMockTest, presignInnerV4_additionalSignedHeader) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;
    config.additionalHeaders = {"Abc"};

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto datetime = utils::FormatUnixTime(expiration, "%Y%m%dT%H%M%SZ");
    auto date = datetime.substr(0, 8);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(4LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));
    EXPECT_EQ("abc-value1", output->signedHeaders.at("abc"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos,
              output->url.find("x-oss-credential=ak%2F" + date + "%2Fcn-hangzhou%2Foss%2Faliyun_v4_request"));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-date=" + date));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-signature-version=OSS4-HMAC-SHA256"));
    EXPECT_TRUE(output->url.find("x-oss-expires=3599") != std::string::npos ||
                output->url.find("x-oss-expires=3600") != std::string::npos);

    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));
}


TEST(ClientImplMockTest, presignInnerV4_token) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk", "token+1");
    config.httpTransport = mockHandler;
    config.additionalHeaders = {"Abc"};

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto datetime = utils::FormatUnixTime(expiration, "%Y%m%dT%H%M%SZ");
    auto date = datetime.substr(0, 8);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(4LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));
    EXPECT_EQ("abc-value1", output->signedHeaders.at("abc"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos,
              output->url.find("x-oss-credential=ak%2F" + date + "%2Fcn-hangzhou%2Foss%2Faliyun_v4_request"));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-date=" + date));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-signature-version=OSS4-HMAC-SHA256"));
    EXPECT_TRUE(output->url.find("x-oss-expires=3599") != std::string::npos ||
                output->url.find("x-oss-expires=3600") != std::string::npos);
    EXPECT_NE(std::string::npos, output->url.find("x-oss-security-token=token%2B1"));

    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));
}

TEST(ClientImplMockTest, presignInnerV4_defaultExpiration) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 15 * 60LL;
    // input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto datetime = utils::FormatUnixTime(expiration, "%Y%m%dT%H%M%SZ");
    auto date = datetime.substr(0, 8);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_TRUE(output->expiration < expiration + 10);
    EXPECT_TRUE(output->expiration > expiration - 10);
    EXPECT_EQ(0LL, output->signedHeaders.size());
    EXPECT_NE(std::string::npos, output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?"));
    EXPECT_NE(std::string::npos,
              output->url.find("x-oss-credential=ak%2F" + date + "%2Fcn-hangzhou%2Foss%2Faliyun_v4_request"));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-date=" + date));
    EXPECT_NE(std::string::npos, output->url.find("x-oss-signature-version=OSS4-HMAC-SHA256"));
    EXPECT_TRUE(output->url.find("x-oss-expires=900") != std::string::npos ||
                output->url.find("x-oss-expires=899") != std::string::npos);
}

TEST(ClientImplMockTest, presignInnerV1) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    // no headers & parameters
    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    std::int64_t expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(0LL, output->signedHeaders.size());
    EXPECT_NE(std::string::npos, output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?"));
    EXPECT_NE(std::string::npos, output->url.find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, output->url.find("Expires="));
    EXPECT_NE(std::string::npos, output->url.find("Signature="));

    // default signed headers
    mockHandler->Clear();

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    result = client.Presign(input);
    output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(3LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos, output->url.find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, output->url.find("Expires=" + std::to_string(expiration)));
    EXPECT_NE(std::string::npos, output->url.find("Signature="));
    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));

    // additional headers - abc should not be included in signedHeaders
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.additionalHeaders = {"Abc"};
    config.httpTransport = mockHandler;

    client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    result = client.Presign(input);
    output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(3LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos, output->url.find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, output->url.find("Expires=" + std::to_string(expiration)));
    EXPECT_NE(std::string::npos, output->url.find("Signature="));
    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));

    // token
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk", "token+1");
    config.httpTransport = mockHandler;

    client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key+123/subdir/1.txt";
    input.headers = {
            {"Content-Type", "text"}, {"Content-MD5", "md5-123"}, {"x-oss-meta-key1", "value1"},
            {"abc", "abc-value1"},    {"abc-2", "abc-value2"},
    };
    input.parameters = {{"key#?+", "value#123/+123"}, {"key", "value"}};

    expiration = static_cast<std::int64_t>(std::time(nullptr));
    expiration += 60LL * 60LL;
    input.opMetadata.emplace("EXPIRATION_TIME", expiration);

    result = client.Presign(input);
    output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(expiration, output->expiration);
    EXPECT_EQ(3LL, output->signedHeaders.size());
    EXPECT_EQ("text", output->signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-123", output->signedHeaders.at("Content-MD5"));
    EXPECT_EQ("value1", output->signedHeaders.at("x-oss-meta-key1"));

    EXPECT_NE(std::string::npos,
              output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key%2B123/subdir/1.txt?"));
    EXPECT_NE(std::string::npos, output->url.find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, output->url.find("Expires=" + std::to_string(expiration)));
    EXPECT_NE(std::string::npos, output->url.find("Signature="));
    EXPECT_NE(std::string::npos, output->url.find("security-token=token%2B1"));
    EXPECT_NE(std::string::npos, output->url.find("key=value"));
    EXPECT_NE(std::string::npos, output->url.find("key%23%3F%2B=value%23123%2F%2B123"));
}


TEST(ClientImplMockTest, presignInnerV1_defaultExpiration) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();

    // no headers & parameters
    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Presign(input);
    auto output = std::get_if<PresignInnerOutput>(&result);
    EXPECT_NE(nullptr, output);

    EXPECT_EQ("GET", output->method);
    EXPECT_EQ(0LL, output->signedHeaders.size());
    EXPECT_NE(std::string::npos, output->url.find("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?"));
    EXPECT_NE(std::string::npos, output->url.find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, output->url.find("Expires="));
    EXPECT_NE(std::string::npos, output->url.find("Signature="));
    EXPECT_NE(0, output->expiration);

    std::int64_t expectedExpiration = static_cast<std::int64_t>(std::time(nullptr)) + 15 * 60;
    EXPECT_LT(output->expiration, expectedExpiration + 10);
    EXPECT_GT(output->expiration, expectedExpiration - 10);
}

TEST(ClientImplMockTest, presignMisc) {
    // empty ak&sk

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("", "");

    auto client = ClientImpl(config, defaultClientFns);

    auto input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Presign(input);
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("CredentialsError", error->getCode());
    EXPECT_EQ("Credentials is null or empty.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("", error->getSnapshot());

    // Null CredentialsProvider
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";

    client = ClientImpl(config, defaultClientFns);

    input = OperationInput{};
    input.opName = "GetObject";
    input.method = "GET";
    input.bucket = "bucket";
    input.key = "key";

    result = client.Presign(input);
    error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(0, error->getStatusCode());
    EXPECT_EQ("", error->getRequestId());
    EXPECT_EQ("IllegalArgument", error->getCode());
    EXPECT_EQ("Credentials provider is null.", error->getMessage());
    EXPECT_EQ("", error->getEC());
    EXPECT_EQ("", error->getSnapshot());
}

TEST(ClientImplMockTest, returnsServiceExceptionNormal_json) {}

TEST(ClientImplMockTest, returnsServiceExceptionEmpty_json) {}

TEST(ClientImplMockTest, returnsServiceExceptionInvalidFormat_json) {}


// ---------------------------------------------------------------------------
// Clock Skew Correction Tests
// ---------------------------------------------------------------------------

const static std::string ClockSkewErrorXml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<Error>
    <Code>RequestTimeTooSkewed</Code>
    <Message>The difference between the request time and the current time is too large.</Message>
    <RequestId>id-skew</RequestId>
    <ServerTime>2018-03-07T08:35:19.000Z</ServerTime>
    <MaxAllowedSkewMilliseconds>900000</MaxAllowedSkewMilliseconds>
</Error>
)";

const static std::string AccessDeniedErrorXml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<Error>
    <Code>AccessDenied</Code>
    <Message>Access Denied.</Message>
    <RequestId>id-denied</RequestId>
</Error>
)";

TEST(ClientImplMockTest, ClockSkew_RetryWithCorrectedTime) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    auto now = std::time(nullptr);
    auto serverDateStr = utils::ToGmtTime(now);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(ClockSkewErrorXml)}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}},
            std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
}

TEST(ClientImplMockTest, ClockSkew_OffsetPersistsAcrossRequests) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    auto serverTime = std::time(nullptr) + 600;
    auto serverDateStr = utils::ToGmtTime(serverTime);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(ClockSkewErrorXml)}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}},
            std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);

    // Second request should use stored offset (no clock skew error)
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-3"}},
            std::make_shared<std::stringstream>("")}));

    result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);

    // Verify the stored clock offset is > 0 (approx 600s)
    EXPECT_GT(client.getInnerOptions().clockOffset, 500);
}

TEST(ClientImplMockTest, ClockSkew_DisabledFlag) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableClockSkewCorrection = true;

    auto client = ClientImpl(config, defaultClientFns);

    auto serverTime = std::time(nullptr) + 600;
    auto serverDateStr = utils::ToGmtTime(serverTime);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(ClockSkewErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("RequestTimeTooSkewed", error->getCode());
}

TEST(ClientImplMockTest, ClockSkew_NonSkewError403) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}, {"Date", "Wed, 07 Mar 2018 08:35:19 GMT"}},
            std::make_shared<std::stringstream>(AccessDeniedErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("AccessDenied", error->getCode());
}

TEST(ClientImplMockTest, ClockSkew_ServerTimeFallback) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // No Date header, but ServerTime is in the XML body
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}},
            std::make_shared<std::stringstream>(ClockSkewErrorXml)}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}},
            std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
}

TEST(ClientImplMockTest, ClockSkew_ParseFailure) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    // RequestTimeTooSkewed but no Date header and no ServerTime in body
    std::string errorXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Error>
    <Code>RequestTimeTooSkewed</Code>
    <Message>The difference between the request time and the current time is too large.</Message>
    <RequestId>id-skew</RequestId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}},
            std::make_shared<std::stringstream>(errorXml)}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("RequestTimeTooSkewed", error->getCode());
}

TEST(ClientImplMockTest, ClockSkew_OneShotBodyNotRetried) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    auto serverTime = std::time(nullptr) + 600;
    auto serverDateStr = utils::ToGmtTime(serverTime);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(ClockSkewErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";
    input.body = std::make_shared<StreamContent>(std::make_shared<std::istringstream>("hello"), false);

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
}


const static std::string InvalidSigningDateErrorXml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<Error>
    <Code>InvalidArgument</Code>
    <Message>Invalid signing date in Authorization header.</Message>
    <RequestId>id-invalid-date</RequestId>
</Error>
)";

TEST(ClientImplMockTest, ClockSkew_InvalidSigningDate_LargeSkew) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    auto serverTime = std::time(nullptr) + 1200;
    auto serverDateStr = utils::ToGmtTime(serverTime);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            400, "Bad Request",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(InvalidSigningDateErrorXml)}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}},
            std::make_shared<std::stringstream>("")}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(2ULL, mockHandler->requests.size());
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
}

TEST(ClientImplMockTest, ClockSkew_InvalidSigningDate_SmallSkew) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    auto serverTime = std::time(nullptr) + 60;
    auto serverDateStr = utils::ToGmtTime(serverTime);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            400, "Bad Request",
            {{"x-oss-request-id", "id-1"}, {"Date", serverDateStr}},
            std::make_shared<std::stringstream>(InvalidSigningDateErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input);
    EXPECT_EQ(1ULL, mockHandler->requests.size());
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(400, error->getStatusCode());
    EXPECT_EQ("InvalidArgument", error->getCode());
}


// ---------------------------------------------------------------------------
// Pre/Post ServiceError Split Tests
// ---------------------------------------------------------------------------

TEST(ClientImplMockTest, PrePostServiceError_ErrorStillParsed) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(ErrorXml)}));

    auto input = OperationInput{};
    input.opName = "PutBucketAcl";
    input.method = "PUT";
    input.bucket = "bucket";

    auto result = client.Execute(input);
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
    EXPECT_EQ("InvalidAccessKeyId", error->getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error->getMessage());
}

TEST(ClientImplMockTest, PrePostServiceError_SuccessHandlersBlockedOnError) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>(ErrorXml)}));

    bool handlerCalled = false;
    OperationInnerOptions innerOpts;
    innerOpts.onResponseMessage.emplace_back(
        [&handlerCalled](std::unique_ptr<ResponseMessage>&, ExecuteContext&) {
            handlerCalled = true;
            return true;
        });

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_FALSE(handlerCalled);
    auto error = std::get_if<OperationError>(&result);
    EXPECT_NE(nullptr, error);
    EXPECT_EQ(403, error->getStatusCode());
}

TEST(ClientImplMockTest, PrePostServiceError_SuccessHandlersRunOn2xx) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>("")}));

    bool handlerCalled = false;
    OperationInnerOptions innerOpts;
    innerOpts.onResponseMessage.emplace_back(
        [&handlerCalled](std::unique_ptr<ResponseMessage>&, ExecuteContext&) {
            handlerCalled = true;
            return true;
        });

    auto input = OperationInput{};
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    auto result = client.Execute(input, nullptr, &innerOpts);
    EXPECT_TRUE(handlerCalled);
    auto output = std::get_if<OperationOutput>(&result);
    EXPECT_NE(nullptr, output);
    EXPECT_EQ(200, output->statusCode);
}


// ---------------------------------------------------------------------------
// Feature Flags Configuration Tests
// ---------------------------------------------------------------------------

TEST(ClientImplMockTest, FeatureFlags_DefaultAllEnabled) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_DisableClockSkewCorrection) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableClockSkewCorrection = true;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_DisableAutoDetectMimeType) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableAutoDetectMimeType = true;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_DisableUploadCRC64Check) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableUploadCRC64Check = true;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_DisableDownloadCRC64Check) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableDownloadCRC64Check = true;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_DisableMultiple) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableClockSkewCorrection = true;
    config.disableUploadCRC64Check = true;
    config.disableDownloadCRC64Check = true;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

TEST(ClientImplMockTest, FeatureFlags_ExplicitFalseKeepsEnabled) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableClockSkewCorrection = false;
    config.disableAutoDetectMimeType = false;

    auto client = ClientImpl(config, defaultClientFns);

    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDownload));
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud