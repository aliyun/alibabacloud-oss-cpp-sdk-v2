#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSClientObjectSymlinkCallAsyncTest, PutSymlink_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-sym-1"}},
                nullptr}));

    auto request = models::PutSymlinkRequest();
    request.setBucket("test-bucket");
    request.setKey("symlink-key");
    request.setSymlinkTarget("target-key");

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("id-sym-1", outcome.value().getRequestId());
}

TEST(OSSClientObjectSymlinkCallAsyncTest, GetSymlink_Callback_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-sym-2"},
                 {"x-oss-symlink-target", "target-key"}},
                nullptr}));

    auto request = models::GetSymlinkRequest();
    request.setBucket("test-bucket");
    request.setKey("symlink-key");

    std::promise<GetSymlinkOutcome> promise;
    auto future = promise.get_future();

    client.asyncCallback(request,
        [&promise](const OSSClient*, const models::GetSymlinkRequest&, const GetSymlinkOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("target-key", outcome.value().getSymlinkTarget());
}

TEST(OSSClientObjectSymlinkCallAsyncTest, PutSymlink_Future_NoExecutor) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = std::make_shared<MockTransport>();
    auto client = OSSClient(config);

    auto request = models::PutSymlinkRequest();
    request.setBucket("test-bucket");
    request.setKey("symlink-key");
    request.setSymlinkTarget("target-key");

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoExecutor", outcome.error().getCode());
}

} // namespace alibabacloud::oss2
