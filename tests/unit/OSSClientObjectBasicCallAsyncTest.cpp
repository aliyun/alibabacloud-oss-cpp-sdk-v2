#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSClientObjectBasicCallAsyncTest, PutObject_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-hash-crc64ecma", "123456789"},
                 {"x-oss-version-id", "version123"},
                 {"x-oss-request-id", "id-1234"}},
                nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("test data"));

    auto future = client.callAsync<PutObjectOutcome>(&OSSClient::putObject, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
    EXPECT_EQ("id-1234", outcome.getResult().getRequestId());
}

TEST(OSSClientObjectBasicCallAsyncTest, PutObject_Callback_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-5678"}},
                nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("test data"));

    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    client.callAsync<PutObjectOutcome>(&OSSClient::putObject, request,
        [&promise](const OSSClient*, const models::PutObjectRequest&, const PutObjectOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("id-5678", outcome.getResult().getRequestId());
}

TEST(OSSClientObjectBasicCallAsyncTest, PutObject_Future_NoExecutor) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<PutObjectOutcome>(&OSSClient::putObject, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("NoExecutor", outcome.getError().getCode());
}

TEST(OSSClientObjectBasicCallAsyncTest, PutObject_Callback_NoExecutor) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    auto client = OSSClient(config);

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    client.callAsync<PutObjectOutcome>(&OSSClient::putObject, request,
        [&promise](const OSSClient*, const models::PutObjectRequest&, const PutObjectOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("NoExecutor", outcome.getError().getCode());
}

TEST(OSSClientObjectBasicCallAsyncTest, GetObject_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-get-1"},
                 {"Content-Length", "11"},
                 {"ETag", "\"etag123\""}},
                std::make_shared<std::stringstream>("hello world")}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<GetObjectOutcome>(&OSSClient::getObject, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

TEST(OSSClientObjectBasicCallAsyncTest, HeadObject_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-head-1"},
                 {"Content-Length", "100"},
                 {"ETag", "\"etag456\""}},
                nullptr}));

    auto request = models::HeadObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<HeadObjectOutcome>(&OSSClient::headObject, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

TEST(OSSClientObjectBasicCallAsyncTest, DeleteObject_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content",
                {{"x-oss-request-id", "id-del-1"}},
                nullptr}));

    auto request = models::DeleteObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<DeleteObjectOutcome>(&OSSClient::deleteObject, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientObjectBasicCallAsyncTest, CopyObject_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<CopyObjectResult>
  <ETag>"etag-copy"</ETag>
  <LastModified>2024-01-01T00:00:00.000Z</LastModified>
</CopyObjectResult>)";

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-copy-1"}},
                std::make_shared<std::stringstream>(body)}));

    auto request = models::CopyObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("dest-key");
    request.setSourceKey("src-key");

    auto future = client.callAsync<CopyObjectOutcome>(&OSSClient::copyObject, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientObjectBasicCallAsyncTest, PutObject_Future_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>AccessDenied</Code>
    <Message>Access denied</Message>
    <RequestId>id-err-1</RequestId>
</Error>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-err-1"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(std::make_shared<StringContent>("data"));

    auto future = client.callAsync<PutObjectOutcome>(&OSSClient::putObject, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("AccessDenied", outcome.getError().getCode());
}

} // namespace alibabacloud::oss2
