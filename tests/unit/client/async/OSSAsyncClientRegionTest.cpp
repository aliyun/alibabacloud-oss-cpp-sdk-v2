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

TEST(OSSAsyncClientRegionTest, DescribeRegionsAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<RegionInfoList>
  <RegionInfo>
    <Region>oss-cn-hangzhou</Region>
    <InternetEndpoint>oss-cn-hangzhou.aliyuncs.com</InternetEndpoint>
    <InternalEndpoint>oss-cn-hangzhou-internal.aliyuncs.com</InternalEndpoint>
    <AccelerateEndpoint>oss-accelerate.aliyuncs.com</AccelerateEndpoint>
  </RegionInfo>
</RegionInfoList>
    )";
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DescribeRegionsRequest();
    auto future = client.callAsync<DescribeRegionsOutcome>(&OSSAsyncClient::describeRegionsAsync, request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(200, outcome.getResult().getStatusCode());
    EXPECT_EQ("id-1234", outcome.getResult().getRequestId());
    EXPECT_EQ(1, outcome.getResult().getRegionInfoList().regionInfos.size());
    EXPECT_EQ("oss-cn-hangzhou", outcome.getResult().getRegionInfoList().regionInfos.at(0).region);
}

TEST(OSSAsyncClientRegionTest, DescribeRegionsAsync_ErrorResponse) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
</Error>)";
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DescribeRegionsRequest();
    auto future = client.callAsync<DescribeRegionsOutcome>(&OSSAsyncClient::describeRegionsAsync, request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ(403, outcome.getError().getStatusCode());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace alibabacloud::oss2
