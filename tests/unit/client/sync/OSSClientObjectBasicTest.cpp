#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

namespace {

} // namespace

// Test PutObject operation
TEST(OSSClientObjectBasicTest, PutObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-version-id", "version123"},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("test data for put object"));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("version123", result.getVersionId());
}

TEST(OSSClientObjectBasicTest, PutObject_WithHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-version-id", "version123"},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setForbidOverwrite("true");
    request.setStorageClass("IA");
    request.setTagging("tag1=value1&tag2=value2");
    request.setServerSideEncryption("AES256");
    request.setObjectAcl("private");

    HeaderCollection metadata;
    metadata["x-oss-meta-custom"] = "custom-value";
    request.setMetadata(metadata);

    request.setBody(std::make_shared<StringContent>("test data for put object with headers"));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("version123", result.getVersionId());
}

TEST(OSSClientObjectBasicTest, PutObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>AccessDenied</Code>
    <Message>Access denied</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("test data"));

    auto outcome = client.putObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("PutObject", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("AccessDenied", error.getCode());
    EXPECT_EQ("Access denied", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, PutObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::PutObjectRequest();
    auto outcome = client.putObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.putObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test GetObject operation
TEST(OSSClientObjectBasicTest, GetObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    std::string responseBody = "This is the content of the test object.";
    auto responseStream = std::make_shared<std::stringstream>(responseBody);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200,
                            "OK",
                            {{"Content-Type", "text/plain"},
                             {"ETag", "\"test-etag\""},
                             {"Last-Modified", "Mon, 01 Jan 2023 00:00:00 GMT"},
                             {"x-oss-request-id", "id-1234"},
                             {"Content-Length", std::to_string(responseBody.length())}},
                            responseStream}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.getObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("text/plain", result.getContentType());
    EXPECT_EQ("\"test-etag\"", result.getETag());
    EXPECT_EQ("Mon, 01 Jan 2023 00:00:00 GMT", result.getLastModified());
    EXPECT_EQ(responseBody.length(), static_cast<size_t>(result.getContentLength()));

    auto body = result.getBody();
    ASSERT_NE(nullptr, body);
    std::istreambuf_iterator<char> isb(*body.get()), end;
    std::string content(isb, end);
    *body >> content;
    EXPECT_EQ(responseBody, content);
}

TEST(OSSClientObjectBasicTest, GetObject_WithRange) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto responseBody = "partial content";
    auto responseStream = std::make_shared<std::stringstream>(responseBody);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{206,
                                                              "Partial Content",
                                                              {{"Content-Range", "bytes 0-13/100"},
                                                               {"Content-Type", "text/plain"},
                                                               {"ETag", "\"test-etag\""},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              responseStream}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setRange("bytes=0-13");

    auto outcome = client.getObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(206, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("text/plain", result.getContentType());
    EXPECT_EQ("\"test-etag\"", result.getETag());

    auto body = result.getBody();
    ASSERT_NE(nullptr, body);
    std::istreambuf_iterator<char> isb(*body.get()), end;
    std::string content(isb, end);
    EXPECT_EQ(responseBody, content);
}

TEST(OSSClientObjectBasicTest, GetObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("non-existent-key");

    auto outcome = client.getObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("GetObject", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/non-existent-key", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, GetObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::GetObjectRequest();
    auto outcome = client.getObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.getObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());
}

TEST(OSSClientObjectBasicTest, GetObject_WithSinkFactory) {
    class ContextCaptureMock : public HttpTransport {
      public:
        ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
            capturedOptions = options;
            return std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK",
                    {{"x-oss-request-id", "id-5678"}, {"Content-Length", "11"}},
                    std::make_shared<std::stringstream>("hello world")});
        }
        std::string getName() const override { return "ContextCaptureMock"; }
        RequestOptions capturedOptions;
    };

    auto mockHandler = std::make_shared<ContextCaptureMock>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    SinkFactory factory;
    factory.supplier = [](std::int64_t size, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(std::make_shared<std::stringstream>());
    };
    factory.isOneShot = true;

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key").setSinkFactory(factory);

    auto outcome = client.getObject(request);
    EXPECT_TRUE(outcome.has_value());
    ASSERT_TRUE(mockHandler->capturedOptions.sinkFactory.has_value());
    EXPECT_TRUE(mockHandler->capturedOptions.sinkFactory->isOneShot);
    EXPECT_NE(nullptr, mockHandler->capturedOptions.sinkFactory->supplier);
}

TEST(OSSClientObjectBasicTest, GetObject_WithoutSinkFactory) {
    class ContextCaptureMock : public HttpTransport {
      public:
        ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
            capturedOptions = options;
            return std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK",
                    {{"x-oss-request-id", "id-5678"}, {"Content-Length", "5"}},
                    std::make_shared<std::stringstream>("hello")});
        }
        std::string getName() const override { return "ContextCaptureMock"; }
        RequestOptions capturedOptions;
    };

    auto mockHandler = std::make_shared<ContextCaptureMock>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");

    auto outcome = client.getObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(mockHandler->capturedOptions.sinkFactory.has_value());
}

// Test CopyObject operation
TEST(OSSClientObjectBasicTest, CopyObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyObjectResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-copy-source-version-id", "src-version123"},
                                                               {"x-oss-version-id", "dst-version123"}},
                                                              std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key");
    request.setCopySource("/source-bucket/source-key");

    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("dst-version123", result.getVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());
}

TEST(OSSClientObjectBasicTest, CopyObject_WithHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyObjectResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key");
    request.setCopySource("/source-bucket/source-key");
    request.setStorageClass("IA");
    request.setObjectAcl("public-read");
    request.setServerSideEncryption("AES256");
    request.setTagging("tag1=value1");
    request.setMetadataDirective("REPLACE");

    HeaderCollection metadata;
    metadata["x-oss-meta-custom"] = "custom-value";
    request.setMetadata(metadata);

    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
}

TEST(OSSClientObjectBasicTest, CopyObject_SourceKey) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyObjectResult>
    )";

    // Set Source Key only
    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-copy-source-version-id", "src-version123"},
                                                               {"x-oss-version-id", "dst-version123"}},
                                                              std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceKey("source-key:123");

    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("dst-version123", result.getVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());

    EXPECT_EQ("/dest-bucket/source-key%3A123", mockHandler->lastRequest->headers.at("x-oss-copy-source"));

    // Set Source Bucket & Key
    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-copy-source-version-id", "src-version123"},
                                                               {"x-oss-version-id", "dst-version123"}},
                                                              std::make_shared<std::stringstream>(body)}));

    request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key:123");

    outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("dst-version123", result.getVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());

    EXPECT_EQ("/source-bucket/source-key%3A123", mockHandler->lastRequest->headers.at("x-oss-copy-source"));

    // Set Source Bucket & Key & versionId
    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-copy-source-version-id", "src-version123"},
                                                               {"x-oss-version-id", "dst-version123"}},
                                                              std::make_shared<std::stringstream>(body)}));

    request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key:123");
    request.setSourceVersionId("id-123");

    outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("dst-version123", result.getVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());

    EXPECT_EQ("/source-bucket/source-key%3A123?versionId=id-123", mockHandler->lastRequest->headers.at("x-oss-copy-source"));

    // Set From CopySource
    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-copy-source-version-id", "src-version123"},
                                                               {"x-oss-version-id", "dst-version123"}},
                                                              std::make_shared<std::stringstream>(body)}));

    request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setCopySource("/source-key/source-key%3A1234");

    outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("dst-version123", result.getVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());

    EXPECT_EQ("/source-key/source-key%3A1234", mockHandler->lastRequest->headers.at("x-oss-copy-source"));
}

TEST(OSSClientObjectBasicTest, CopyObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("source-bucket");
    request.setSourceKey("non-existent-key");
    request.setCopySource("/source-bucket/non-existent-key");

    auto outcome = client.copyObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("CopyObject", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://dest-bucket.oss-cn-hangzhou.aliyuncs.com/dest-key", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, CopyObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyObjectResult>
    )";
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    auto outcome = client.copyObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("dest-bucket");
    outcome = client.copyObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("dest-key");
    outcome = client.copyObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field SourceKey or CopySource", error.getMessage());

    request.setKey("dest-key");
    request.setSourceKey("src-key");
    outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test Append Object operation
TEST(OSSClientObjectBasicTest, AppendObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);


    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-hash-crc64ecma", "14741617095266562575"},
                                                               {"x-oss-next-append-position", "123"}},
                                                              nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPosition(11);
    request.setBody(RequestBody::fromString("hello"));

    auto outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("14741617095266562575", result.getHashCrc64ecma());
    EXPECT_EQ(123, result.getNextAppendPosition());
}

TEST(OSSClientObjectBasicTest, AppendObject_WithHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-hash-crc64ecma", "14741617095266562575"},
                                                               {"x-oss-next-append-position", "123"}},
                                                              nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPosition(0);
    request.setStorageClass("IA");
    request.setObjectAcl("public-read");
    request.setServerSideEncryption("AES256");

    HeaderCollection metadata;
    metadata["custom"] = "custom-value";
    request.setMetadata(metadata);
    request.setBody(RequestBody::fromString("hello"));

    auto outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("14741617095266562575", result.getHashCrc64ecma());
    EXPECT_EQ(123, result.getNextAppendPosition());

    EXPECT_NE(nullptr, mockHandler->lastRequest);
    EXPECT_EQ("custom-value", mockHandler->lastRequest->headers.at("x-oss-meta-custom"));
}

TEST(OSSClientObjectBasicTest, AppendObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::AppendObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPosition(0);

    auto outcome = client.appendObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("AppendObject", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://dest-bucket.oss-cn-hangzhou.aliyuncs.com/dest-key?append&position=0", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, AppendObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::AppendObjectRequest();
    auto outcome = client.appendObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("dest-bucket");
    outcome = client.appendObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("dest-key");
    outcome = client.appendObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Position", error.getMessage());

    request.setPosition(0);
    outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test SealAppendObject operation
TEST(OSSClientObjectBasicTest, SealAppendObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);


    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}, {"x-oss-sealed-time", "12345"}}, nullptr}));

    auto request = models::SealAppendObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPosition(11);

    auto outcome = client.sealAppendObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("12345", result.getSealedTime());
}

TEST(OSSClientObjectBasicTest, SealAppendObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::SealAppendObjectRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPosition(11);

    auto outcome = client.sealAppendObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("SealAppendObject", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://dest-bucket.oss-cn-hangzhou.aliyuncs.com/dest-key?position=11&seal", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, SealAppendObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::SealAppendObjectRequest();
    auto outcome = client.sealAppendObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("dest-bucket");
    outcome = client.sealAppendObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("dest-key");
    outcome = client.sealAppendObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Position", error.getMessage());

    request.setPosition(0);
    outcome = client.sealAppendObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test HeadObject operation
TEST(OSSClientObjectBasicTest, HeadObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"Content-Type", "text/plain"},
                                                               {"Content-Length", "1024"},
                                                               {"ETag", "\"test-etag\""},
                                                               {"Last-Modified", "Mon, 01 Jan 2023 00:00:00 GMT"},
                                                               {"x-oss-storage-class", "Standard"},
                                                               {"x-oss-server-side-encryption", "AES256"},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              nullptr}));

    auto request = models::HeadObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.headObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("text/plain", result.getContentType());
    EXPECT_EQ(1024, result.getContentLength());
    EXPECT_EQ("\"test-etag\"", result.getETag());
    EXPECT_EQ("Mon, 01 Jan 2023 00:00:00 GMT", result.getLastModified());
    EXPECT_EQ("Standard", result.getStorageClass());
    EXPECT_EQ("AES256", result.getServerSideEncryption());
}

TEST(OSSClientObjectBasicTest, HeadObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::HeadObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("non-existent-key");

    auto outcome = client.headObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("HeadObject", error.getOpName());
    EXPECT_EQ("HEAD", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/non-existent-key", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, HeadObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::HeadObjectRequest();
    auto outcome = client.headObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.headObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.headObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test DeleteObject operation
TEST(OSSClientObjectBasicTest, DeleteObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            204,
            "No Content",
            {{"x-oss-request-id", "id-1234"}, {"x-oss-delete-marker", "false"}, {"x-oss-version-id", "version123"}},
            nullptr}));

    auto request = models::DeleteObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.deleteObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("false", result.getDeleteMarker());
    EXPECT_EQ("version123", result.getVersionId());
}

TEST(OSSClientObjectBasicTest, DeleteObject_WithVersionId) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            204, "No Content", {{"x-oss-request-id", "id-1234"}, {"x-oss-delete-marker", "true"}}, nullptr}));

    auto request = models::DeleteObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setVersionId("version-to-delete");

    auto outcome = client.deleteObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("true", result.getDeleteMarker());
}

TEST(OSSClientObjectBasicTest, DeleteObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("non-existent-key");

    auto outcome = client.deleteObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("DeleteObject", error.getOpName());
    EXPECT_EQ("DELETE", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/non-existent-key", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, DeleteObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::DeleteObjectRequest();
    auto outcome = client.deleteObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.deleteObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.deleteObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test GetObjectMeta operation
TEST(OSSClientObjectBasicTest, GetObjectMeta_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"ETag", "\"meta-test-etag\""},
                                                               {"Content-Length", "2048"},
                                                               {"x-oss-last-access-time", "2023-01-01T12:00:00.000Z"},
                                                               {"Last-Modified", "2023-01-01T12:00:00.000Z"},
                                                               {"x-oss-version-id", "version123"},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              nullptr}));

    auto request = models::GetObjectMetaRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.getObjectMeta(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"meta-test-etag\"", result.getETag());
    EXPECT_EQ(2048, result.getContentLength());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastAccessTime());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());
    EXPECT_EQ("version123", result.getVersionId());
}

TEST(OSSClientObjectBasicTest, GetObjectMeta_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectMetaRequest();
    request.setBucket("test-bucket");
    request.setKey("non-existent-key");

    auto outcome = client.getObjectMeta(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("GetObjectMeta", error.getOpName());
    EXPECT_EQ("HEAD", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/non-existent-key?objectMeta", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, GetObjectMeta_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::GetObjectMetaRequest();
    auto outcome = client.getObjectMeta(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.getObjectMeta(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.getObjectMeta(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test RestoreObject operation
TEST(OSSClientObjectBasicTest, RestoreObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{202,
                                                              "Accepted",
                                                              {{"x-oss-request-id", "id-1234"},
                                                               {"x-oss-object-restore-priority", "Standard"},
                                                               {"x-oss-version-id", "version123"}},
                                                              nullptr}));

    auto request = models::RestoreObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::RestoreRequest restoreReq;
    restoreReq.setDays(7);
    models::JobParameters jobParams;
    jobParams.setTier("Standard");
    restoreReq.setJobParameters(jobParams);
    request.setRestoreRequest(restoreReq);

    auto outcome = client.restoreObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(202, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("Standard", result.getObjectRestorePriority());
    EXPECT_EQ("version123", result.getVersionId());
}

TEST(OSSClientObjectBasicTest, RestoreObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidObjectState</Code>
    <Message>The operation is not valid for the current object state</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            409, "Conflict", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::RestoreObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::RestoreRequest restoreReq;
    restoreReq.setDays(7);
    request.setRestoreRequest(restoreReq);

    auto outcome = client.restoreObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(409, error.getStatusCode());
    EXPECT_EQ("RestoreObject", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?restore", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidObjectState", error.getCode());
    EXPECT_EQ("The operation is not valid for the current object state", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, RestoreObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::RestoreObjectRequest();
    auto outcome = client.restoreObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.restoreObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.restoreObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test RestoreObject operation
TEST(OSSClientObjectBasicTest, CleanRestoredObject_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            202, "Accepted", {{"x-oss-request-id", "id-1234"}, {"x-oss-version-id", "version123"}}, nullptr}));

    auto request = models::CleanRestoredObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.cleanRestoredObject(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(202, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientObjectBasicTest, CleanRestoredObject_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidObjectState</Code>
    <Message>The operation is not valid for the current object state</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            409, "Conflict", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CleanRestoredObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.cleanRestoredObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(409, error.getStatusCode());
    EXPECT_EQ("CleanRestoredObject", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?cleanRestoredObject",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidObjectState", error.getCode());
    EXPECT_EQ("The operation is not valid for the current object state", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, CleanRestoredObject_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::CleanRestoredObjectRequest();
    auto outcome = client.cleanRestoredObject(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.cleanRestoredObject(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.cleanRestoredObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// Test DeleteMultipleObjects operation
TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
    <Deleted>
       <Key>multipart.data</Key>
    </Deleted>
    <Deleted>
       <Key>test.jpg</Key>
    </Deleted>
    <Deleted>
       <Key>demo.jpg</Key>
    </Deleted>
</DeleteResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            204,
            "No Content",
            {{"x-oss-request-id", "id-1234"}, {"x-oss-version-id", "version123"}},
            std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(models::Delete{{models::ObjectIdentifier{"key1&1"}, {models::ObjectIdentifier{"key2", "id-2"}}}});

    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ(3, result.getDeletedObjects().size());
    EXPECT_EQ("multipart.data", result.getDeletedObjects().at(0).key);
    EXPECT_EQ("test.jpg", result.getDeletedObjects().at(1).key);
    EXPECT_EQ("demo.jpg", result.getDeletedObjects().at(2).key);
    EXPECT_EQ("", result.getEncodingType());

    auto src = mockHandler->lastRequest->body->spanSource();
    auto data = src->readToEnd();
    std::string str(data.begin(), data.end());
    EXPECT_EQ("<Delete><Object><Key>key1&amp;1</Key></Object><Object><Key>key2</Key><VersionId>id-2</VersionId></Object></Delete>", str);
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
    <Deleted>
       <Key>folder%2Fmultipart.data</Key>
       <DeleteMarker>true</DeleteMarker>
       <DeleteMarkerVersionId>id-1</DeleteMarkerVersionId>
    </Deleted>
    <Deleted>
       <Key>demo.jpg</Key>
       <VersionId>id-2</VersionId>
       <DeleteMarker>true</DeleteMarker>
       <DeleteMarkerVersionId>id-3</DeleteMarkerVersionId>
    </Deleted>
    <EncodingType>url</EncodingType>
</DeleteResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            204,
            "No Content",
            {{"x-oss-request-id", "id-1234"}, {"x-oss-version-id", "version123"}},
            std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(models::Delete{{models::ObjectIdentifier{"key1&1"}, {models::ObjectIdentifier{"key2", "id-2"}}}});

    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ(2, result.getDeletedObjects().size());
    EXPECT_EQ("folder/multipart.data", result.getDeletedObjects().at(0).key);
    EXPECT_EQ(false, result.getDeletedObjects().at(0).versionId.has_value());
    EXPECT_EQ(true, result.getDeletedObjects().at(0).deleteMarker);
    EXPECT_EQ("id-1", result.getDeletedObjects().at(0).deleteMarkerVersionId);
    EXPECT_EQ("demo.jpg", result.getDeletedObjects().at(1).key);
    EXPECT_EQ("id-2", result.getDeletedObjects().at(1).versionId);
    EXPECT_EQ(true, result.getDeletedObjects().at(1).deleteMarker);
    EXPECT_EQ("id-3", result.getDeletedObjects().at(1).deleteMarkerVersionId);
}


TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
    <Deleted>
       <Key>folder%2Fmultipart.data</Key>
       <DeleteMarker>true</DeleteMarker>
       <DeleteMarkerVersionId>id-1</DeleteMarkerVersionId>
    </Deleted>
    <Deleted>
       <Key>demo.jpg</Key>
       <VersionId>id-2</VersionId>
       <DeleteMarker>true</DeleteMarker>
       <DeleteMarkerVersionId>id-3</DeleteMarkerVersionId>
    </Deleted>
    <EncodingType></EncodingType>
</DeleteResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            204,
            "No Content",
            {{"x-oss-request-id", "id-1234"}, {"x-oss-version-id", "version123"}},
            std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(models::Delete{{models::ObjectIdentifier{"key1&1"}, {models::ObjectIdentifier{"key2", "id-2"}}}});

    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ(2, result.getDeletedObjects().size());
    EXPECT_EQ("folder%2Fmultipart.data", result.getDeletedObjects().at(0).key);
    EXPECT_EQ(false, result.getDeletedObjects().at(0).versionId.has_value());
    EXPECT_EQ(true, result.getDeletedObjects().at(0).deleteMarker);
    EXPECT_EQ("id-1", result.getDeletedObjects().at(0).deleteMarkerVersionId);
    EXPECT_EQ("demo.jpg", result.getDeletedObjects().at(1).key);
    EXPECT_EQ("id-2", result.getDeletedObjects().at(1).versionId);
    EXPECT_EQ(true, result.getDeletedObjects().at(1).deleteMarker);
    EXPECT_EQ("id-3", result.getDeletedObjects().at(1).deleteMarkerVersionId);
}


TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(models::Delete{{models::ObjectIdentifier{"multipart.data"}}});

    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("DeleteMultipleObjects", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/?delete&encoding-type=url", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchKey", error.getCode());
    EXPECT_EQ("The specified key does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
    <Deleted>
       <Key>multipart.data</Key>
    </Deleted>
</DeleteResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DeleteMultipleObjectsRequest();
    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.deleteMultipleObjects(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Delete", error.getMessage());

    request.setDelete(models::Delete{{models::ObjectIdentifier{"multipart.data"}}});
    outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, PutObject_Progress) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    std::string data(1024, 'A');
    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;

    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(data.size()), total);
    };

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>(data));
    request.setProgressCallback(progress);

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);
}

// --- CRC64 Upload Check Tests: PutObject ---

TEST(OSSClientObjectBasicTest, PutObject_CRC64Check_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "Hello, OSS!";
    uint64_t crc = utils::CalcCRC64(0, data.data(), data.size());

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", std::to_string(crc)}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, mockHandler->requests.size());
}

TEST(OSSClientObjectBasicTest, PutObject_CRC64Check_Mismatch) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "Hello, OSS!";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-3"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-4"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.putObject(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
}

TEST(OSSClientObjectBasicTest, PutObject_CRC64Check_Disabled) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableUploadCRC64Check = true;

    auto client = OSSClient(config);

    std::string data = "Hello, OSS!";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, PutObject_CRC64Check_NoServerCRC) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "Hello, OSS!";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, PutObject_CRC64Check_NoBody) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
}

// --- CRC64 Upload Check Tests: AppendObject ---

TEST(OSSClientObjectBasicTest, AppendObject_CRC64Check_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "hello";
    uint64_t crc = utils::CalcCRC64(0, data.data(), data.size());

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", std::to_string(crc)},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPosition(0);
    request.setInitHashCRC64(0);
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(std::to_string(crc), outcome.value().getHashCrc64ecma());
}

TEST(OSSClientObjectBasicTest, AppendObject_CRC64Check_Mismatch) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "hello";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", "99999"},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPosition(0);
    request.setInitHashCRC64(0);
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.appendObject(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
    EXPECT_EQ(1ULL, mockHandler->requests.size());
}

TEST(OSSClientObjectBasicTest, AppendObject_CRC64Check_NoInitCRC) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "hello";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", "99999"},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPosition(0);
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, AppendObject_CRC64Check_Disabled) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.disableUploadCRC64Check = true;

    auto client = OSSClient(config);

    std::string data = "hello";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", "99999"},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPosition(0);
    request.setInitHashCRC64(0);
    request.setBody(RequestBody::fromString(data));

    auto outcome = client.appendObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, HeadObject_WithAllHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK",
                           {{"x-oss-request-id", "id-head"},
                            {"Content-Length", "1024"},
                            {"Content-Type", "text/plain"},
                            {"ETag", "\"etag-123\""},
                            {"Last-Modified", "Mon, 01 Jan 2024 00:00:00 GMT"},
                            {"x-oss-object-type", "Normal"},
                            {"x-oss-storage-class", "Standard"},
                            {"x-oss-hash-crc64ecma", "12345678"}},
                           nullptr}));

    auto request = models::HeadObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.headObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSClientObjectBasicTest, GetObjectMeta_WithAllHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK",
                           {{"x-oss-request-id", "id-meta"},
                            {"Content-Length", "512"},
                            {"ETag", "\"etag-456\""},
                            {"Last-Modified", "Tue, 02 Jan 2024 00:00:00 GMT"},
                            {"x-oss-hash-crc64ecma", "87654321"},
                            {"x-oss-version-id", "vid-001"}},
                           nullptr}));

    auto request = models::GetObjectMetaRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.getObjectMeta(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSClientObjectBasicTest, RestoreObject_WithRestoreRequest) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{202, "Accepted", {{"x-oss-request-id", "id-restore"}}, nullptr}));

    auto request = models::RestoreObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    models::RestoreRequest rr;
    rr.days = 3;
    models::JobParameters jp;
    jp.tier = "Standard";
    rr.jobParameters = jp;
    request.setRestoreRequest(rr);
    auto outcome = client.restoreObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, RestoreObject_WithEmptyRestoreRequest) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{202, "Accepted", {{"x-oss-request-id", "id-restore2"}}, nullptr}));

    auto request = models::RestoreObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    models::RestoreRequest rr;
    request.setRestoreRequest(rr);
    auto outcome = client.restoreObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_WithQuietAndVersionId) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
</DeleteResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-dm2"}},
                           std::make_shared<std::stringstream>(body)}));

    models::Delete del;
    del.setQuiet(true);
    del.setObjects({models::ObjectIdentifier{"f1.txt", "vid-1"},
                    models::ObjectIdentifier{"f2.txt"}});
    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(del);
    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_WithDeletedItems) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<DeleteResult>
    <EncodingType>url</EncodingType>
    <Deleted>
        <Key>file1.txt</Key>
        <VersionId>vid-001</VersionId>
        <DeleteMarker>true</DeleteMarker>
        <DeleteMarkerVersionId>vid-dm-001</DeleteMarkerVersionId>
    </Deleted>
    <Deleted>
        <Key>file2.txt</Key>
    </Deleted>
</DeleteResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-dm"}},
                           std::make_shared<std::stringstream>(body)}));

    models::Delete del;
    del.setObjects({models::ObjectIdentifier{"file1.txt"}, models::ObjectIdentifier{"file2.txt"}});
    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(del);
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_NullBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    models::Delete del;
    del.setObjects({models::ObjectIdentifier{"file1.txt"}});
    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(del);
    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, DeleteMultipleObjects_InvalidXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}},
                           std::make_shared<std::stringstream>("<bad")}));

    models::Delete del;
    del.setObjects({models::ObjectIdentifier{"file1.txt"}});
    auto request = models::DeleteMultipleObjectsRequest();
    request.setBucket("test-bucket");
    request.setDelete(del);
    auto outcome = client.deleteMultipleObjects(request);
    EXPECT_FALSE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, CopyObject_WithVersionHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
    <LastModified>2024-01-01T00:00:00.000Z</LastModified>
    <ETag>"etag-copy"</ETag>
</CopyObjectResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK",
                           {{"x-oss-request-id", "id-copy"},
                            {"x-oss-version-id", "vid-new"},
                            {"x-oss-copy-source-version-id", "vid-src"},
                            {"x-oss-hash-crc64ecma", "12345"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setSourceKey("source-key");
    request.setSourceBucket("source-bucket");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, CopyObject_MinimalResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
</CopyObjectResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-copy-min"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setSourceKey("source-key");
    request.setSourceBucket("source-bucket");
    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, CopyObject_NullBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::CopyObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setSourceKey("source-key");
    request.setSourceBucket("source-bucket");
    auto outcome = client.copyObject(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectBasicTest, PutObject_WithCallback) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    std::string callbackBody = R"({"Status":"OK"})";
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1234"}},
            std::make_shared<std::stringstream>(callbackBody)}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setCallback("callback-base64-data");
    request.setCallbackVar("callbackvar-base64-data");
    request.setBody(std::make_shared<StringContent>("hello"));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(callbackBody, outcome.value().getCallbackResult());

    EXPECT_EQ("callback-base64-data", mockHandler->lastRequest->headers.at("x-oss-callback"));
    EXPECT_EQ("callbackvar-base64-data", mockHandler->lastRequest->headers.at("x-oss-callback-var"));
}

TEST(OSSClientObjectBasicTest, PutObject_WithoutCallback_EmptyCallbackResult) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200,
                                                              "OK",
                                                              {{"x-oss-version-id", "version123"},
                                                               {"x-oss-request-id", "id-1234"}},
                                                              nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("test data"));

    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("", outcome.value().getCallbackResult());
}

} // namespace alibabacloud::oss2
