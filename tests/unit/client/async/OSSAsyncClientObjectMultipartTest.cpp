#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "CRC64MockAsyncTransport.h"
#include "MockAsyncTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSAsyncClientObjectMultipartTest, InitiateMultipartUploadAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::InitiateMultipartUploadRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field Key", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, InitiateMultipartUploadAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <UploadId>upload-id-123</UploadId>
</InitiateMultipartUploadResult>)";
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket").setKey("test-key");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("upload-id-123", outcome.value().getUploadId());
}

TEST(OSSAsyncClientObjectMultipartTest, UploadPartAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::UploadPartRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket").setKey("test-key").setUploadId("uid");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field PartNumber", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, CompleteMultipartUploadAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::CompleteMultipartUploadRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field UploadId", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, UploadPartCopyAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::UploadPartCopyRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, AbortMultipartUploadAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::AbortMultipartUploadRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, ListMultipartUploadsAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::ListMultipartUploadsRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, ListPartsAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::ListPartsRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field UploadId", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectMultipartTest, UploadPartAsync_Progress) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"ETag", "\"etag123\""}, {"x-oss-request-id", "id-1234"}}, nullptr}));

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
    request.setBucket("test-bucket").setKey("test-key");
    request.setPartNumber(1).setUploadId("upload-id-123");
    request.setBody(std::make_shared<StringContent>(data));
    request.setProgressCallback(progress);

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());

    // MockAsyncTransport doesn't read body, drain it to trigger observers
    if (mockTransport->lastRequest && mockTransport->lastRequest->body) {
        auto src = mockTransport->lastRequest->body->spanSource();
        if (src) { src->readToEnd(); }
    }

    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);
}

// --- CRC64 Upload Check Tests: Async UploadPart ---

TEST(OSSAsyncClientObjectMultipartTest, UploadPartAsync_CRC64Check_Success) {
    auto mockTransport = std::make_shared<CRC64MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    std::string data = "Hello, OSS!";
    uint64_t crc = utils::CalcCRC64(0, data.data(), data.size());

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"ETag", "\"etag-1\""},
             {"x-oss-hash-crc64ecma", std::to_string(crc)}},
            nullptr}));

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-123");
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, mockTransport->requests.size());
}

TEST(OSSAsyncClientObjectMultipartTest, UploadPartAsync_CRC64Check_Mismatch) {
    auto mockTransport = std::make_shared<CRC64MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    std::string data = "Hello, OSS!";

    for (int i = 0; i < 4; ++i) {
        mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
                200, "OK",
                {{"x-oss-request-id", "id-1"}, {"ETag", "\"etag-1\""}, {"x-oss-hash-crc64ecma", "99999"}},
                nullptr}));
    }

    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("upload-123");
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
}

} // namespace alibabacloud::oss2
