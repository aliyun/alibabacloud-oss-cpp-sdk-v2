#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/models/ObjectMultipart.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSClientObjectMultipartCallAsyncTest, InitiateMultipartUpload_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
  <Bucket>test-bucket</Bucket>
  <Key>test-key</Key>
  <UploadId>upload-id-123</UploadId>
</InitiateMultipartUploadResult>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-mp-1"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<InitiateMultipartUploadOutcome>(&OSSClient::initiateMultipartUpload, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("upload-id-123", outcome.getResult().getUploadId());
}

TEST(OSSClientObjectMultipartCallAsyncTest, AbortMultipartUpload_Callback_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content",
                {{"x-oss-request-id", "id-mp-2"}},
                nullptr}));

    auto request = models::AbortMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-id-123");

    std::promise<AbortMultipartUploadOutcome> promise;
    auto future = promise.get_future();

    client.callAsync<AbortMultipartUploadOutcome>(&OSSClient::abortMultipartUpload, request,
        [&promise](const OSSClient*, const models::AbortMultipartUploadRequest&, const AbortMultipartUploadOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientObjectMultipartCallAsyncTest, ListMultipartUploads_Future_Success) {
    auto mockHandler = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;
    config.executor = std::make_shared<DefaultExecutor>();
    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListMultipartUploadsResult>
  <Bucket>test-bucket</Bucket>
  <IsTruncated>false</IsTruncated>
</ListMultipartUploadsResult>)";

    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-mp-3"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListMultipartUploadsRequest();
    request.setBucket("test-bucket");

    auto future = client.callAsync<ListMultipartUploadsOutcome>(&OSSClient::listMultipartUploads, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientObjectMultipartCallAsyncTest, InitiateMultipartUpload_Future_NoExecutor) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = std::make_shared<MockTransport>();
    auto client = OSSClient(config);

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto future = client.callAsync<InitiateMultipartUploadOutcome>(&OSSClient::initiateMultipartUpload, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("NoExecutor", outcome.getError().getCode());
}

} // namespace alibabacloud::oss2
