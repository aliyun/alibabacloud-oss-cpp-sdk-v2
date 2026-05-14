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

TEST(OSSAsyncClientObjectSymlinkTest, PutSymlinkAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutSymlinkRequest();
    auto future = client.callAsync<PutSymlinkOutcome>(&OSSAsyncClient::putSymlinkAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());

    request.setBucket("test-bucket");
    auto future2 = client.callAsync<PutSymlinkOutcome>(&OSSAsyncClient::putSymlinkAsync, request);
    auto outcome2 = future2.get();
    EXPECT_FALSE(outcome2.isSuccess());
    EXPECT_EQ("Missing field Key", outcome2.getError().getMessage());
}

TEST(OSSAsyncClientObjectSymlinkTest, PutSymlinkAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutSymlinkRequest();
    request.setBucket("test-bucket").setKey("test-link").setSymlinkTarget("test-target");
    auto future = client.callAsync<PutSymlinkOutcome>(&OSSAsyncClient::putSymlinkAsync, request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

TEST(OSSAsyncClientObjectSymlinkTest, GetSymlinkAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetSymlinkRequest();
    auto future = client.callAsync<GetSymlinkOutcome>(&OSSAsyncClient::getSymlinkAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

} // namespace alibabacloud::oss2
