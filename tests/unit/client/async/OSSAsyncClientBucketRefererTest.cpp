#include <gtest/gtest.h>

#include <sstream>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "MockAsyncTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSAsyncClientBucketRefererTest, PutBucketRefererAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutBucketRefererRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncClientBucketRefererTest, PutBucketRefererAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketRefererRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncClientBucketRefererTest, GetBucketRefererAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketRefererRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncClientBucketRefererTest, PutBucketRefererAsync_WithBlacklist) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-bl"}}, nullptr}));

    auto request = models::PutBucketRefererRequest();
    request.setBucket("test-bucket");

    models::RefererConfiguration refererConfig;
    refererConfig.setAllowEmptyReferer(false);
    refererConfig.setAllowTruncateQueryString(true);
    refererConfig.setTruncatePath(false);

    models::RefererList refererList;
    refererList.referers.push_back("https://example.com");
    refererList.referers.push_back("https://test.com");
    refererConfig.setRefererList(refererList);

    models::RefererBlacklist blacklist;
    blacklist.referers.push_back("https://bad.com");
    blacklist.referers.push_back("https://evil.com");
    refererConfig.setRefererBlacklist(blacklist);

    request.setRefererConfiguration(refererConfig);

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSAsyncClientBucketRefererTest, GetBucketRefererAsync_WithBlacklist) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    std::string xmlBody =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<RefererConfiguration>"
        "<AllowEmptyReferer>false</AllowEmptyReferer>"
        "<AllowTruncateQueryString>true</AllowTruncateQueryString>"
        "<TruncatePath>false</TruncatePath>"
        "<RefererList>"
        "<Referer>https://example.com</Referer>"
        "</RefererList>"
        "<RefererBlacklist>"
        "<Referer>https://bad.com</Referer>"
        "<Referer>https://evil.com</Referer>"
        "</RefererBlacklist>"
        "</RefererConfiguration>";

    auto bodyStream = std::make_shared<std::stringstream>(xmlBody);
    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-get-bl"}}, bodyStream}));

    auto request = models::GetBucketRefererRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_TRUE(result.hasRefererConfiguration());
    auto& rc = result.getRefererConfiguration();
    EXPECT_TRUE(rc.refererBlacklist.has_value());
    EXPECT_EQ(2u, rc.refererBlacklist->referers.size());
    EXPECT_EQ("https://bad.com", rc.refererBlacklist->referers[0]);
    EXPECT_EQ("https://evil.com", rc.refererBlacklist->referers[1]);
}

} // namespace alibabacloud::oss2
