#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/models/ObjectMultipart.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {
// Test InitiateMultipartUpload operation
TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key</Key>
  <UploadId>1234567890abcdef</UploadId>
</InitiateMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("1234567890abcdef", result.getUploadId());
    EXPECT_EQ("test-bucket", result.getBucket());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F123</Key>
  <UploadId>1234567890abcdef</UploadId>
  <EncodingType>url</EncodingType>
</InitiateMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("test-key/123", result.getKey());
    EXPECT_EQ("1234567890abcdef", result.getUploadId());
    EXPECT_EQ("test-bucket", result.getBucket());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F123</Key>
  <UploadId>1234567890abcdef</UploadId>
</InitiateMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("test-key%2F123", result.getKey());
    EXPECT_EQ("1234567890abcdef", result.getUploadId());
    EXPECT_EQ("test-bucket", result.getBucket());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_WithHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key</Key>
  <UploadId>1234567890abcdef</UploadId>
</InitiateMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setStorageClass("IA");
    request.setTagging("tag1=value1&tag2=value2");
    request.setServerSideEncryption("AES256");

    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("1234567890abcdef", result.getUploadId());
    EXPECT_EQ("test-bucket", result.getBucket());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
    <OSSAccessKeyId>ak</OSSAccessKeyId>
    <EC>0002-00000902</EC>
    <RecommendDoc>https://api.aliyun.com/troubleshoot?q=0002-00000902</RecommendDoc>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("InitiateMultipartUpload", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?encoding-type=url&uploads", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
    EXPECT_EQ("0002-00000902", error.getEC());
    EXPECT_EQ(7, error.getErrorFields().size());
    EXPECT_EQ("ak", error.getErrorFields().at("OSSAccessKeyId"));
    EXPECT_EQ(1, error.getHeaders().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::InitiateMultipartUploadRequest();
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.initiateMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());
}

// Test UploadPart operation
TEST(OSSClientObjectMultipartTest, UploadPart_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"ETag", "\"test-etag\""}, {"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");

    request.setBody(RequestBody::FromString("test data for upload part"));

    auto outcome = client.uploadPart(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("\"test-etag\"", result.getETag());
}

TEST(OSSClientObjectMultipartTest, UploadPart_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>EntityTooSmall</Code>
    <Message>Part size must be greater than 100K</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            400, "Bad Request", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");

    request.setBody(RequestBody::FromString("too small data"));

    auto outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(400, error.getStatusCode());
    EXPECT_EQ("UploadPart", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?partNumber=1&uploadId=test-upload-id",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("EntityTooSmall", error.getCode());
    EXPECT_EQ("Part size must be greater than 100K", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, UploadPart_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::UploadPartRequest();
    auto outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field PartNumber", error.getMessage());

    request.setPartNumber(1);
    outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field UploadId", error.getMessage());
}

// Test UploadPart operation
TEST(OSSClientObjectMultipartTest, UploadPartCopy_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyPartResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyPartResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartCopyRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");
    request.setCopySource("source-bucket/source-key");

    auto outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
}

TEST(OSSClientObjectMultipartTest, UploadPartCopy_SourceKey) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyPartResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyPartResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartCopyRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");
    request.setSourceKey("source-key:123");

    auto outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());

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

    request = models::UploadPartCopyRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");    
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key:123");

    outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
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

    request = models::UploadPartCopyRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");        
    request.setSourceBucket("source-bucket");
    request.setSourceKey("source-key:123");
    request.setSourceVersionId("id-123");

    outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
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

    request = models::UploadPartCopyRequest();
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");           
    request.setCopySource("/source-key/source-key%3A1234");

    outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
    result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("\"copied-etag\"", result.getETag());
    EXPECT_EQ("2023-01-01T12:00:00.000Z", result.getLastModified());

    EXPECT_EQ("/source-key/source-key%3A1234", mockHandler->lastRequest->headers.at("x-oss-copy-source"));
}

TEST(OSSClientObjectMultipartTest,  UploadPartCopy_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>EntityTooSmall</Code>
    <Message>Part size must be greater than 100K</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            400, "Bad Request", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models:: UploadPartCopyRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");
    request.setSourceKey("src-key");
    request.setSourceBucket("src-bucket");

    auto outcome = client. uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(400, error.getStatusCode());
    EXPECT_EQ("UploadPartCopy", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?partNumber=1&uploadId=test-upload-id",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("EntityTooSmall", error.getCode());
    EXPECT_EQ("Part size must be greater than 100K", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}


TEST(OSSClientObjectMultipartTest, UploadPartCopy_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyPartResult>
  <LastModified>2023-01-01T12:00:00.000Z</LastModified>
  <ETag>"copied-etag"</ETag>
</CopyPartResult>
    )";


    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartCopyRequest();
    auto outcome = client.uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field PartNumber", error.getMessage());

    request.setPartNumber(1);
    outcome = client.uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field UploadId", error.getMessage());

    request.setUploadId("1");
    outcome = client.uploadPartCopy(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field SourceKey or CopySource", error.getMessage());

    request.setSourceKey("src-key");
    outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
}


// Test CompleteMultipartUpload operation
TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
  <Location>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key</Location>
  <Bucket>test-bucket</Bucket>
  <Key>test-key</Key>
  <ETag>"complete-etag"</ETag>
</CompleteMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    // Set up parts to complete the upload
    models::CompleteMultipartUpload completeUpload;
    models::Part part1;
    part1.setETag("etag1").setPartNumber(1);
    models::Part part2;
    part2.setETag("etag2").setPartNumber(2);
    std::vector<models::Part> parts = {part1, part2};
    completeUpload.setParts(parts);
    request.setCompleteMultipartUpload(completeUpload);

    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"complete-etag\"", result.getETag());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key", result.getLocation());
    EXPECT_EQ("", result.getEncodingType());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
  <Location>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key</Location>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F123</Key>
  <ETag>"complete-etag"</ETag>
  <EncodingType>url</EncodingType>
</CompleteMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    // Set up parts to complete the upload
    models::CompleteMultipartUpload completeUpload;
    models::Part part1;
    part1.setETag("etag1").setPartNumber(1);
    models::Part part2;
    part2.setETag("etag2").setPartNumber(2);
    std::vector<models::Part> parts = {part1, part2};
    completeUpload.setParts(parts);
    request.setCompleteMultipartUpload(completeUpload);

    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"complete-etag\"", result.getETag());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key/123", result.getKey());
    EXPECT_EQ("http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key", result.getLocation());
    EXPECT_EQ("url", result.getEncodingType());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
  <Location>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key</Location>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F123</Key>
  <ETag>"complete-etag"</ETag>
  <EncodingType></EncodingType>
</CompleteMultipartUploadResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    // Set up parts to complete the upload
    models::CompleteMultipartUpload completeUpload;
    models::Part part1;
    part1.setETag("etag1").setPartNumber(1);
    models::Part part2;
    part2.setETag("etag2").setPartNumber(2);
    std::vector<models::Part> parts = {part1, part2};
    completeUpload.setParts(parts);
    request.setCompleteMultipartUpload(completeUpload);

    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("\"complete-etag\"", result.getETag());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key%2F123", result.getKey());
    EXPECT_EQ("http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key", result.getLocation());
    EXPECT_EQ("", result.getEncodingType());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidPart</Code>
    <Message>One or more of the specified parts could not be found</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            400, "Bad Request", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    models::CompleteMultipartUpload completeUpload;
    models::Part part;
    part.setETag("invalid-etag").setPartNumber(999);
    std::vector<models::Part> parts = {part};
    completeUpload.setParts(parts);
    request.setCompleteMultipartUpload(completeUpload);

    auto outcome = client.completeMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(400, error.getStatusCode());
    EXPECT_EQ("CompleteMultipartUpload", error.getOpName());
    EXPECT_EQ("POST", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?encoding-type=url&uploadId=test-upload-id",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidPart", error.getCode());
    EXPECT_EQ("One or more of the specified parts could not be found", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::CompleteMultipartUploadRequest();
    auto outcome = client.completeMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.completeMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.completeMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field UploadId", error.getMessage());
}

// Test AbortMultipartUpload operation
TEST(OSSClientObjectMultipartTest, AbortMultipartUpload_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::AbortMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    auto outcome = client.abortMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientObjectMultipartTest, AbortMultipartUpload_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchUpload</Code>
    <Message>The specified upload does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::AbortMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("invalid-upload-id");

    auto outcome = client.abortMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("AbortMultipartUpload", error.getOpName());
    EXPECT_EQ("DELETE", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?uploadId=invalid-upload-id",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchUpload", error.getCode());
    EXPECT_EQ("The specified upload does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, AbortMultipartUpload_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::AbortMultipartUploadRequest();
    auto outcome = client.abortMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.abortMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.abortMultipartUpload(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field UploadId", error.getMessage());
}

// Test ListMultipartUploads operation
TEST(OSSClientObjectMultipartTest, ListMultipartUploads_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
  <Bucket>test-bucket</Bucket>
  <KeyMarker></KeyMarker>
  <UploadIdMarker></UploadIdMarker>
  <NextKeyMarker></NextKeyMarker>
  <NextUploadIdMarker></NextUploadIdMarker>
  <Delimiter></Delimiter>
  <Prefix></Prefix>
  <MaxUploads>1000</MaxUploads>
  <IsTruncated>false</IsTruncated>
  <Upload>
    <Key>test-object-1</Key>
    <UploadId>upload-id-1</UploadId>
    <Initiated>2023-01-01T12:00:00.000Z</Initiated>
  </Upload>
  <Upload>
    <Key>test-object-2</Key>
    <UploadId>upload-id-2</UploadId>
    <Initiated>2023-01-02T12:00:00.000Z</Initiated>
  </Upload>
</ListMultipartUploadsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");

    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ(2, result.getUploads().size());
    EXPECT_EQ("test-object-1", result.getUploads()[0].key);
    EXPECT_EQ("upload-id-1", result.getUploads()[0].uploadId);
    EXPECT_EQ("test-object-2", result.getUploads()[1].key);
    EXPECT_EQ("upload-id-2", result.getUploads()[1].uploadId);
    EXPECT_EQ(false, result.getIsTruncated());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
  <Bucket>test-bucket</Bucket>
  <KeyMarker>123%2F%3A</KeyMarker>
  <UploadIdMarker></UploadIdMarker>
  <NextKeyMarker>123%2F1234</NextKeyMarker>
  <NextUploadIdMarker></NextUploadIdMarker>
  <Delimiter>%2F</Delimiter>
  <Prefix>123%2F</Prefix>
  <MaxUploads>1000</MaxUploads>
  <IsTruncated>false</IsTruncated>
  <EncodingType>url</EncodingType>
  <Upload>
    <Key>123%2Ftest-object-1</Key>
    <UploadId>upload-id-1</UploadId>
    <Initiated>2023-01-01T12:00:00.000Z</Initiated>
  </Upload>
  <Upload>
    <Key>123%2Ftest-object-2%2F456</Key>
    <UploadId>upload-id-2</UploadId>
    <Initiated>2023-01-02T12:00:00.000Z</Initiated>
  </Upload>
</ListMultipartUploadsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");

    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("123/:", result.getKeyMarker());
    EXPECT_EQ("123/1234", result.getNextKeyMarker());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ("123/", result.getPrefix());
    EXPECT_EQ(2, result.getUploads().size());
    EXPECT_EQ("123/test-object-1", result.getUploads()[0].key);
    EXPECT_EQ("upload-id-1", result.getUploads()[0].uploadId);
    EXPECT_EQ("123/test-object-2/456", result.getUploads()[1].key);
    EXPECT_EQ("upload-id-2", result.getUploads()[1].uploadId);
    EXPECT_EQ(false, result.getIsTruncated());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
  <Bucket>test-bucket</Bucket>
  <KeyMarker>123%2F%3A</KeyMarker>
  <UploadIdMarker></UploadIdMarker>
  <NextKeyMarker>123%2F1234</NextKeyMarker>
  <NextUploadIdMarker></NextUploadIdMarker>
  <Delimiter>%2F</Delimiter>
  <Prefix>123%2F</Prefix>
  <MaxUploads>1000</MaxUploads>
  <IsTruncated>false</IsTruncated>
  <EncodingType></EncodingType>
  <Upload>
    <Key>123%2Ftest-object-1</Key>
    <UploadId>upload-id-1</UploadId>
    <Initiated>2023-01-01T12:00:00.000Z</Initiated>
  </Upload>
  <Upload>
    <Key>123%2Ftest-object-2%2F456</Key>
    <UploadId>upload-id-2</UploadId>
    <Initiated>2023-01-02T12:00:00.000Z</Initiated>
  </Upload>
</ListMultipartUploadsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");

    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("123%2F%3A", result.getKeyMarker());
    EXPECT_EQ("123%2F1234", result.getNextKeyMarker());
    EXPECT_EQ("%2F", result.getDelimiter());
    EXPECT_EQ("123%2F", result.getPrefix());
    EXPECT_EQ(2, result.getUploads().size());
    EXPECT_EQ("123%2Ftest-object-1", result.getUploads()[0].key);
    EXPECT_EQ("upload-id-1", result.getUploads()[0].uploadId);
    EXPECT_EQ("123%2Ftest-object-2%2F456", result.getUploads()[1].key);
    EXPECT_EQ("upload-id-2", result.getUploads()[1].uploadId);
    EXPECT_EQ(false, result.getIsTruncated());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_ErrorResponse) {
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

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");

    auto outcome = client.listMultipartUploads(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListMultipartUploads", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/?encoding-type=url&uploads", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("AccessDenied", error.getCode());
    EXPECT_EQ("Access denied", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::ListMultipartUploadsRequest();
    auto outcome = client.listMultipartUploads(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());
}

// Test ListParts operation
TEST(OSSClientObjectMultipartTest, ListParts_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListPartsResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key</Key>
  <UploadId>test-upload-id</UploadId>
  <PartNumberMarker>0</PartNumberMarker>
  <NextPartNumberMarker>2</NextPartNumberMarker>
  <MaxParts>1000</MaxParts>
  <IsTruncated>false</IsTruncated>
  <Part>
    <PartNumber>1</PartNumber>
    <LastModified>2023-01-01T12:00:00.000Z</LastModified>
    <ETag>"etag-1"</ETag>
    <Size>1048576</Size>
  </Part>
  <Part>
    <PartNumber>2</PartNumber>
    <LastModified>2023-01-01T12:00:01.000Z</LastModified>
    <ETag>"etag-2"</ETag>
    <Size>1048576</Size>
  </Part>
</ListPartsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ(1000, result.getMaxParts());
    EXPECT_EQ(0, result.getPartNumberMarker());
    EXPECT_EQ(2, result.getNextPartNumberMarker());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ(2, result.getParts().size());
    EXPECT_EQ(1, result.getParts()[0].partNumber);
    EXPECT_EQ("\"etag-1\"", result.getParts()[0].eTag);
    EXPECT_EQ(1048576, result.getParts()[0].size);
    EXPECT_EQ(2, result.getParts()[1].partNumber);
    EXPECT_EQ("\"etag-2\"", result.getParts()[1].eTag);
    EXPECT_EQ(1048576, result.getParts()[1].size);
    EXPECT_EQ(false, result.getIsTruncated());
}

TEST(OSSClientObjectMultipartTest, ListParts_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListPartsResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F321</Key>
  <UploadId>test-upload-id</UploadId>
  <PartNumberMarker>0</PartNumberMarker>
  <NextPartNumberMarker>2</NextPartNumberMarker>
  <MaxParts>1000</MaxParts>
  <IsTruncated>true</IsTruncated>
  <EncodingType>url</EncodingType>
  <Part>
    <PartNumber>1</PartNumber>
    <LastModified>2023-01-01T12:00:00.000Z</LastModified>
    <ETag>"etag-1"</ETag>
    <Size>1048576</Size>
  </Part>
  <Part>
    <PartNumber>2</PartNumber>
    <LastModified>2023-01-01T12:00:01.000Z</LastModified>
    <ETag>"etag-2"</ETag>
    <Size>1048576</Size>
  </Part>
</ListPartsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key/321", result.getKey());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ(1, result.getParts()[0].partNumber);
    EXPECT_EQ("\"etag-1\"", result.getParts()[0].eTag);
    EXPECT_EQ(1048576, result.getParts()[0].size);
    EXPECT_EQ(2, result.getParts()[1].partNumber);
    EXPECT_EQ("\"etag-2\"", result.getParts()[1].eTag);
    EXPECT_EQ(1048576, result.getParts()[1].size);
    EXPECT_EQ(true, result.getIsTruncated());
}

TEST(OSSClientObjectMultipartTest, ListParts_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListPartsResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key%2F321</Key>
  <UploadId>test-upload-id</UploadId>
  <PartNumberMarker>0</PartNumberMarker>
  <NextPartNumberMarker>2</NextPartNumberMarker>
  <MaxParts>1000</MaxParts>
  <EncodingType></EncodingType>
  <Part>
    <PartNumber>1</PartNumber>
    <LastModified>2023-01-01T12:00:00.000Z</LastModified>
    <ETag>"etag-1"</ETag>
    <Size>1048576</Size>
  </Part>
  <Part>
    <PartNumber>2</PartNumber>
    <LastModified>2023-01-01T12:00:01.000Z</LastModified>
    <ETag>"etag-2"</ETag>
    <Size>1048576</Size>
  </Part>
</ListPartsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key%2F321", result.getKey());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ(1, result.getParts()[0].partNumber);
    EXPECT_EQ("\"etag-1\"", result.getParts()[0].eTag);
    EXPECT_EQ(1048576, result.getParts()[0].size);
    EXPECT_EQ(2, result.getParts()[1].partNumber);
    EXPECT_EQ("\"etag-2\"", result.getParts()[1].eTag);
    EXPECT_EQ(1048576, result.getParts()[1].size);
    EXPECT_EQ(false, result.getIsTruncated());
}


TEST(OSSClientObjectMultipartTest, ListParts_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchUpload</Code>
    <Message>The specified upload does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("invalid-upload-id");

    auto outcome = client.listParts(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("ListParts", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key?encoding-type=url&uploadId=invalid-upload-id",
              error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("NoSuchUpload", error.getCode());
    EXPECT_EQ("The specified upload does not exist", error.getMessage());
    EXPECT_EQ(4, error.getErrorFields().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectMultipartTest, ListParts_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::ListPartsRequest();
    auto outcome = client.listParts(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.listParts(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());

    request.setKey("test-key");
    outcome = client.listParts(request);
    EXPECT_FALSE(outcome.has_value());
    error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field UploadId", error.getMessage());
}

TEST(OSSClientObjectMultipartTest, UploadPart_Progress) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"ETag", "\"etag123\""}, {"x-oss-request-id", "id-1234"}}, nullptr}));

    std::string data(2048, 'B');
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

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-id-123");
    request.setBody(std::make_shared<StringContent>(data));
    request.setProgressCallback(progress);

    auto outcome = client.uploadPart(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);
}

// --- CRC64 Upload Check Tests: UploadPart ---

TEST(OSSClientObjectMultipartTest, UploadPart_CRC64Check_Success) {
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
            {{"x-oss-request-id", "id-1"},
             {"ETag", "\"etag-1\""},
             {"x-oss-hash-crc64ecma", std::to_string(crc)}},
            nullptr}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-123");
    request.setBody(RequestBody::FromString(data));

    auto outcome = client.uploadPart(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, mockHandler->requests.size());
}

TEST(OSSClientObjectMultipartTest, UploadPart_CRC64Check_Mismatch) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    std::string data = "Hello, OSS!";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-2"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-3"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-4"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-123");
    request.setBody(RequestBody::FromString(data));

    auto outcome = client.uploadPart(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
}

TEST(OSSClientObjectMultipartTest, UploadPart_CRC64Check_Disabled) {
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
            {{"x-oss-request-id", "id-1"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
            nullptr}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-123");
    request.setBody(RequestBody::FromString(data));

    auto outcome = client.uploadPart(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_FullResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
    <Bucket>test-bucket</Bucket>
    <KeyMarker></KeyMarker>
    <UploadIdMarker></UploadIdMarker>
    <NextKeyMarker>key2</NextKeyMarker>
    <NextUploadIdMarker>uid2</NextUploadIdMarker>
    <Delimiter>/</Delimiter>
    <Prefix></Prefix>
    <MaxUploads>100</MaxUploads>
    <IsTruncated>false</IsTruncated>
    <EncodingType>url</EncodingType>
    <Upload>
        <Key>file1.txt</Key>
        <UploadId>upload-001</UploadId>
        <Initiated>2024-01-01T00:00:00.000Z</Initiated>
    </Upload>
    <CommonPrefixes>
        <Prefix>dir/</Prefix>
    </CommonPrefixes>
</ListMultipartUploadsResult>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-mp"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_NullBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");
    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_InvalidXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}},
                           std::make_shared<std::stringstream>("<bad")}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");
    auto outcome = client.listMultipartUploads(request);
    EXPECT_FALSE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListMultipartUploads_MinimalResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
    <Upload>
    </Upload>
</ListMultipartUploadsResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-lmu-min"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");
    auto outcome = client.listMultipartUploads(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListParts_FullResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListPartsResult>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <UploadId>upload-001</UploadId>
    <PartNumberMarker>0</PartNumberMarker>
    <NextPartNumberMarker>3</NextPartNumberMarker>
    <MaxParts>100</MaxParts>
    <IsTruncated>false</IsTruncated>
    <EncodingType>url</EncodingType>
    <Part>
        <PartNumber>1</PartNumber>
        <LastModified>2024-01-01T00:00:00.000Z</LastModified>
        <ETag>"etag-p1"</ETag>
        <Size>1048576</Size>
        <HashCrc64ecma>12345678</HashCrc64ecma>
    </Part>
</ListPartsResult>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-lp"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-001");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListParts_MinimalResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListPartsResult>
    <Part>
    </Part>
</ListPartsResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-lp-min"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-001");
    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, ListParts_NullBody) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::ListPartsRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-001");
    auto outcome = client.listParts(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_WithCustomHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <UploadId>upload-id-999</UploadId>
    <EncodingType>url</EncodingType>
</InitiateMultipartUploadResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-init"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, InitiateMultipartUpload_MinimalResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
</InitiateMultipartUploadResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-init-min"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    auto outcome = client.initiateMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_WithCustomHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
    <Location>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key</Location>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <ETag>"etag-final"</ETag>
    <EncodingType>url</EncodingType>
</CompleteMultipartUploadResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK",
                           {{"x-oss-request-id", "id-comp"},
                            {"x-oss-version-id", "vid-comp"},
                            {"x-oss-hash-crc64ecma", "99999"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-id-999");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_WithParts) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
</CompleteMultipartUploadResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-comp-parts"}},
                           std::make_shared<std::stringstream>(body)}));

    models::CompleteMultipartUpload cmu;
    cmu.parts = {models::Part{"\"etag-1\"", 1}, models::Part{"", 0}};
    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-id-999");
    request.setCompleteMultipartUpload(cmu);
    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, UploadPartCopy_MinimalResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyPartResult>
</CopyPartResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-upc-min"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartCopyRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-id-999");
    request.setSourceKey("source-key");
    request.setSourceBucket("source-bucket");
    auto outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, UploadPartCopy_WithCustomHeaders) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyPartResult>
    <LastModified>2024-01-01T00:00:00.000Z</LastModified>
    <ETag>"etag-part-copy"</ETag>
</CopyPartResult>)";
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-upc"}},
                           std::make_shared<std::stringstream>(body)}));

    auto request = models::UploadPartCopyRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-id-999");
    request.setSourceKey("source-key");
    request.setSourceBucket("source-bucket");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto outcome = client.uploadPartCopy(request);
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectMultipartTest, CompleteMultipartUpload_WithCallback) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    std::string callbackBody = R"({"Status":"OK"})";
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1234"}},
            std::make_shared<std::stringstream>(callbackBody)}));

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-id");
    request.setCallback("callback-base64-data");
    request.setCallbackVar("callbackvar-base64-data");

    auto outcome = client.completeMultipartUpload(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(callbackBody, outcome.value().getCallbackResult());
    EXPECT_EQ("", outcome.value().getBucket());
    EXPECT_EQ("", outcome.value().getKey());

    EXPECT_EQ("callback-base64-data", mockHandler->lastRequest->headers.at("x-oss-callback"));
    EXPECT_EQ("callbackvar-base64-data", mockHandler->lastRequest->headers.at("x-oss-callback-var"));
}

} // namespace alibabacloud::oss2
