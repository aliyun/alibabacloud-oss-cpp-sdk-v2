#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectAppendTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->callAsync<PutBucketOutcome>(&OSSAsyncClient::putBucketAsync,
                                                          models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().isSuccess());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncObjectAppendTest::bucketName_ = "";

TEST_F(AsyncObjectAppendTest, AppendObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-append-object";
    std::string content1 = "Hello, ";
    std::string content2 = "World!";

    auto future1 = client->callAsync<AppendObjectOutcome>(
        &OSSAsyncClient::appendObjectAsync,
        models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(RequestBody::FromString(content1)));
    auto outcome1 = future1.get();
    EXPECT_TRUE(outcome1.isSuccess());
    EXPECT_EQ(content1.size(), outcome1.getResult().getNextAppendPosition());

    auto future2 = client->callAsync<AppendObjectOutcome>(
        &OSSAsyncClient::appendObjectAsync,
        models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(outcome1.getResult().getNextAppendPosition())
            .setBody(RequestBody::FromString(content2)));
    auto outcome2 = future2.get();
    EXPECT_TRUE(outcome2.isSuccess());
    EXPECT_EQ(content1.size() + content2.size(), outcome2.getResult().getNextAppendPosition());
}

TEST_F(AsyncObjectAppendTest, AppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<AppendObjectOutcome>(
        &OSSAsyncClient::appendObjectAsync,
        models::AppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0).setBody(RequestBody::FromString("content")));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectAppendTest, SealAppendObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-seal-append-object";
    std::string content = "Content to seal";

    auto appendFuture = client->callAsync<AppendObjectOutcome>(
        &OSSAsyncClient::appendObjectAsync,
        models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(appendFuture.get().isSuccess());

    auto sealFuture = client->callAsync<SealAppendObjectOutcome>(
        &OSSAsyncClient::sealAppendObjectAsync,
        models::SealAppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(content.size()));
    auto sealOutcome = sealFuture.get();
    if (!sealOutcome.isSuccess()) {
        auto& error = sealOutcome.getError();
        EXPECT_EQ("OperationNotSupported", error.getCode());
    }
}

TEST_F(AsyncObjectAppendTest, SealAppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<SealAppendObjectOutcome>(
        &OSSAsyncClient::sealAppendObjectAsync,
        models::SealAppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
