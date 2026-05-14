#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

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
            if (!responses.empty()) {
                responseResult = std::move(responses.front());
                responses.erase(responses.begin());
            }
        }
        callback(std::move(responseResult), std::move(request), std::move(context));
    }
    std::string getName() const override { return "MockAsyncTransport"; }

    std::vector<std::unique_ptr<ResponseMessage>> responses;
    std::mutex mutex_;
};

} // namespace

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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());

    request.setBucket("test-bucket");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.isSuccess());
    EXPECT_EQ("Missing field Key", outcome2.getError().getMessage());
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

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
    EXPECT_EQ("id-1234", outcome.getResult().getRequestId());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.isSuccess());
    EXPECT_EQ("Missing field SourceKey or CopySource", outcome2.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());

    request.setBucket("test-bucket").setKey("test-key");
    auto future2 = client.asyncCall(request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.isSuccess());
    EXPECT_EQ("Missing field Position", outcome2.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(204, outcome.getResult().getStatusCode());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
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
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

} // namespace alibabacloud::oss2
