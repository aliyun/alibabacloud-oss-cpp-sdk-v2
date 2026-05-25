#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "CRC64MockAsyncTransport.h"
#include "MockAsyncTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSAsyncClientObjectBasicTest, PutObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field Key", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, PutObjectAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1234"}, {"ETag", "\"etag-123\""}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setBody(RequestBody::FromString("hello"));
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("id-1234", outcome.value().getRequestId());
}

TEST(OSSAsyncClientObjectBasicTest, CopyObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::CopyObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field SourceKey or CopySource", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1234"}, {"Content-Length", "5"}},
            std::make_shared<std::stringstream>("hello")}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_WithSinkFactory) {
    class ContextCaptureMockAsync : public AsyncHttpTransport {
      public:
        void sendAsync(std::unique_ptr<RequestMessage> request,
                       const RequestOptions& options,
                       RequestCallback callback) override {
            capturedOptions = options;
            auto response = std::make_unique<ResponseMessage>(ResponseMessage{
                    200, "OK",
                    {{"x-oss-request-id", "id-5678"}, {"Content-Length", "11"}},
                    std::make_shared<std::stringstream>("hello world")});
            callback(std::move(response), std::move(request));
        }
        std::string getName() const override { return "ContextCaptureMockAsync"; }
        RequestOptions capturedOptions;
    };

    auto mockTransport = std::make_shared<ContextCaptureMockAsync>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    SinkFactory factory;
    factory.supplier = [](std::int64_t size) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(std::make_shared<std::stringstream>());
    };
    factory.isOneShot = false;

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key").setSinkFactory(factory);
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    ASSERT_TRUE(mockTransport->capturedOptions.sinkFactory.has_value());
    EXPECT_FALSE(mockTransport->capturedOptions.sinkFactory->isOneShot);
    EXPECT_NE(nullptr, mockTransport->capturedOptions.sinkFactory->supplier);
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_WithoutSinkFactory) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1234"}, {"Content-Length", "5"}},
            std::make_shared<std::stringstream>("hello")}));

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSAsyncClientObjectBasicTest, AppendObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::AppendObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.has_value());
    EXPECT_EQ("Missing field Position", outcome2.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, SealAppendObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::SealAppendObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, DeleteObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::DeleteObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, DeleteObjectAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::DeleteObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(204, outcome.value().getStatusCode());
}

TEST(OSSAsyncClientObjectBasicTest, DeleteMultipleObjectsAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::DeleteMultipleObjectsRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, HeadObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::HeadObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectMetaAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetObjectMetaRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, RestoreObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::RestoreObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, CleanRestoredObjectAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::CleanRestoredObjectRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAsyncClientObjectBasicTest, PutObjectAsync_Progress) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

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
    request.setBucket("test-bucket").setKey("test-key");
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

// --- CRC64 Upload Check Tests: Async PutObject ---

TEST(OSSAsyncClientObjectBasicTest, PutObjectAsync_CRC64Check_Success) {
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
            {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", std::to_string(crc)}},
            nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, mockTransport->requests.size());
}

TEST(OSSAsyncClientObjectBasicTest, PutObjectAsync_CRC64Check_Mismatch) {
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
                {{"x-oss-request-id", "id-1"}, {"x-oss-hash-crc64ecma", "99999"}},
                nullptr}));
    }

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
}

// --- CRC64 Upload Check Tests: Async AppendObject ---

TEST(OSSAsyncClientObjectBasicTest, AppendObjectAsync_CRC64Check_Success) {
    auto mockTransport = std::make_shared<CRC64MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    std::string data = "hello";
    uint64_t crc = utils::CalcCRC64(0, data.data(), data.size());

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", std::to_string(crc)},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setPosition(0);
    request.setInitHashCRC64(0);
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSAsyncClientObjectBasicTest, AppendObjectAsync_CRC64Check_Mismatch) {
    auto mockTransport = std::make_shared<CRC64MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    std::string data = "hello";

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK",
            {{"x-oss-request-id", "id-1"},
             {"x-oss-hash-crc64ecma", "99999"},
             {"x-oss-next-append-position", "5"}},
            nullptr}));

    auto request = models::AppendObjectRequest();
    request.setBucket("test-bucket").setKey("test-key");
    request.setPosition(0);
    request.setInitHashCRC64(0);
    request.setBody(RequestBody::FromString(data));

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
}

} // namespace alibabacloud::oss2
