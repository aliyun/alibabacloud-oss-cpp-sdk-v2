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

TEST(OSSAsyncClientObjectTaggingTest, PutObjectTaggingAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutObjectTaggingRequest();
    auto future = client.callAsync<PutObjectTaggingOutcome>(&OSSAsyncClient::putObjectTaggingAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());

    request.setBucket("test-bucket");
    auto future2 = client.callAsync<PutObjectTaggingOutcome>(&OSSAsyncClient::putObjectTaggingAsync, request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.isSuccess());
    EXPECT_EQ("Missing field Key", outcome2.getError().getMessage());
}

TEST(OSSAsyncClientObjectTaggingTest, GetObjectTaggingAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetObjectTaggingRequest();
    auto future = client.callAsync<GetObjectTaggingOutcome>(&OSSAsyncClient::getObjectTaggingAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

TEST(OSSAsyncClientObjectTaggingTest, DeleteObjectTaggingAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::DeleteObjectTaggingRequest();
    auto future = client.callAsync<DeleteObjectTaggingOutcome>(&OSSAsyncClient::deleteObjectTaggingAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

TEST(OSSAsyncClientObjectTaggingTest, DeleteObjectTaggingAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::DeleteObjectTaggingRequest();
    request.setBucket("test-bucket").setKey("test-key");
    auto future = client.callAsync<DeleteObjectTaggingOutcome>(&OSSAsyncClient::deleteObjectTaggingAsync, request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(204, outcome.getResult().getStatusCode());
}

} // namespace alibabacloud::oss2
