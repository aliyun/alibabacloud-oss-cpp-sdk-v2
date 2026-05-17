#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
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

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_WithOStreamFactory) {
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

    OStreamFactory factory;
    factory.supplier = [](std::int64_t size) {
        return std::make_shared<std::stringstream>();
    };
    factory.isOneShot = false;

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket").setKey("test-key").setOStreamFactory(factory);
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    ASSERT_TRUE(mockTransport->capturedOptions.ostreamFactory.has_value());
    EXPECT_FALSE(mockTransport->capturedOptions.ostreamFactory->isOneShot);
    EXPECT_NE(nullptr, mockTransport->capturedOptions.ostreamFactory->supplier);
}

TEST(OSSAsyncClientObjectBasicTest, GetObjectAsync_WithoutOStreamFactory) {
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

} // namespace alibabacloud::oss2
