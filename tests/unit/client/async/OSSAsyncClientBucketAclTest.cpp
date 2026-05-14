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

TEST(OSSAsyncClientBucketAclTest, PutBucketAclAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutBucketAclRequest();
    auto future = client.callAsync<PutBucketAclOutcome>(&OSSAsyncClient::putBucketAclAsync, request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.getError().getMessage());
}

TEST(OSSAsyncClientBucketAclTest, PutBucketAclAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketAclRequest();
    request.setBucket("test-bucket").setAcl("private");
    auto future = client.callAsync<PutBucketAclOutcome>(&OSSAsyncClient::putBucketAclAsync, request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

TEST(OSSAsyncClientBucketAclTest, GetBucketAclAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketAclRequest();
    auto future = client.callAsync<GetBucketAclOutcome>(&OSSAsyncClient::getBucketAclAsync, request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("ArgumentRequired", outcome.getError().getCode());
}

TEST(OSSAsyncClientBucketAclTest, GetBucketAclAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy><Owner><ID>owner-id</ID></Owner><AccessControlList><Grant>private</Grant></AccessControlList></AccessControlPolicy>)";
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketAclRequest();
    request.setBucket("test-bucket");
    auto future = client.callAsync<GetBucketAclOutcome>(&OSSAsyncClient::getBucketAclAsync, request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
}

} // namespace alibabacloud::oss2
