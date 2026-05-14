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

TEST(OSSAsyncClientBucketBasicTest, GetBucketStatAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketStatRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

TEST(OSSAsyncClientBucketBasicTest, GetBucketStatAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<BucketStat><Storage>1024</Storage><ObjectCount>10</ObjectCount></BucketStat>)";
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketStatRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
    EXPECT_EQ("id-1234", outcome.getResult().getRequestId());
    EXPECT_EQ(1024, outcome.getResult().getBucketStat().storage.value());
}

TEST(OSSAsyncClientBucketBasicTest, PutBucketAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutBucketRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

TEST(OSSAsyncClientBucketBasicTest, PutBucketAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

TEST(OSSAsyncClientBucketBasicTest, DeleteBucketAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::DeleteBucketRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

TEST(OSSAsyncClientBucketBasicTest, DeleteBucketAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::DeleteBucketRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(204, outcome.getResult().getStatusCode());
}

TEST(OSSAsyncClientBucketBasicTest, ListObjectsAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::ListObjectsRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

TEST(OSSAsyncClientBucketBasicTest, ListObjectsV2Async_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::ListObjectsV2Request();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

TEST(OSSAsyncClientBucketBasicTest, GetBucketInfoAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketInfoRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

TEST(OSSAsyncClientBucketBasicTest, GetBucketLocationAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketLocationRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

} // namespace alibabacloud::oss2
