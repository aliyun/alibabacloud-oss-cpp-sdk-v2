#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSClientObjectAclCallAsyncTest, PutObjectAcl_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "id-acl-1"}},
                nullptr}));

    auto request = models::PutObjectAclRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setObjectAcl("private");

    auto future = client.callAsync<PutObjectAclOutcome>(&OSSClient::putObjectAcl, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("id-acl-1", outcome.getResult().getRequestId());
}

TEST(OSSClientObjectAclCallAsyncTest, GetObjectAcl_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy>
  <Owner>
    <ID>1234</ID>
    <DisplayName>owner</DisplayName>
  </Owner>
  <AccessControlList>
    <Grant>private</Grant>
  </AccessControlList>
</AccessControlPolicy>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-acl-2"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<GetObjectAclOutcome>(&OSSClient::getObjectAcl, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("private", outcome.getResult().getAccessControlPolicy().accessControlList.value().grant);
}

TEST(OSSClientObjectAclCallAsyncTest, GetObjectAcl_Callback_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy>
  <AccessControlList>
    <Grant>public-read</Grant>
  </AccessControlList>
</AccessControlPolicy>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-acl-3"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    std::promise<GetObjectAclOutcome> promise;
    auto future = promise.get_future();

    client.callAsync<GetObjectAclOutcome>(&OSSClient::getObjectAcl, request,
        [&promise](const OSSClient*, const models::GetObjectAclRequest&, const GetObjectAclOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("public-read", outcome.getResult().getAccessControlPolicy().accessControlList.value().grant);
}

TEST(OSSClientObjectAclCallAsyncTest, PutObjectAcl_Future_NoExecutor) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = std::make_shared<MockTransport>();
    auto client = OSSClient(config);

    auto request = models::PutObjectAclRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setObjectAcl("private");

    auto future = client.callAsync<PutObjectAclOutcome>(&OSSClient::putObjectAcl, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("NoExecutor", outcome.getError().getCode());
}

} // namespace alibabacloud::oss2
