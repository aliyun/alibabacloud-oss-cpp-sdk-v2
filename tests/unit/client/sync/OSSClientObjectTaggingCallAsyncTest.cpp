#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSClientObjectTaggingCallAsyncTest, PutObjectTagging_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-tag-1"}},
                nullptr}));

    auto request = models::PutObjectTaggingRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("id-tag-1", outcome.value().getRequestId());
}

TEST(OSSClientObjectTaggingCallAsyncTest, GetObjectTagging_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<Tagging>
  <TagSet>
    <Tag>
      <Key>key1</Key>
      <Value>value1</Value>
    </Tag>
  </TagSet>
</Tagging>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-tag-2"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectTaggingRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("id-tag-2", outcome.value().getRequestId());
}

TEST(OSSClientObjectTaggingCallAsyncTest, DeleteObjectTagging_Callback_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content",
                {{"x-oss-request-id", "id-tag-3"}},
                nullptr}));

    auto request = models::DeleteObjectTaggingRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    std::promise<DeleteObjectTaggingOutcome> promise;
    auto future = promise.get_future();

    client.asyncCallback(request,
        [&promise](const OSSClient*, const models::DeleteObjectTaggingRequest&, const DeleteObjectTaggingOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientObjectTaggingCallAsyncTest, GetObjectTagging_Future_NoExecutor) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = std::make_shared<MockTransport>();
    auto client = OSSClient(config);

    auto request = models::GetObjectTaggingRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoExecutor", outcome.error().getCode());
}

} // namespace alibabacloud::oss2
