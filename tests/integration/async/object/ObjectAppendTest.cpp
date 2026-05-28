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
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
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

    auto future1 = client->asyncCall(models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(RequestBody::fromString(content1)));
    auto outcome1 = future1.get();
    EXPECT_TRUE(outcome1.has_value());
    EXPECT_EQ(content1.size(), outcome1.value().getNextAppendPosition());

    auto future2 = client->asyncCall(models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(outcome1.value().getNextAppendPosition())
            .setBody(RequestBody::fromString(content2)));
    auto outcome2 = future2.get();
    EXPECT_TRUE(outcome2.has_value());
    EXPECT_EQ(content1.size() + content2.size(), outcome2.value().getNextAppendPosition());
}

TEST_F(AsyncObjectAppendTest, AppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::AppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0).setBody(RequestBody::fromString("content")));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectAppendTest, SealAppendObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-seal-append-object";
    std::string content = "Content to seal";

    auto appendFuture = client->asyncCall(models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(RequestBody::fromString(content)));
    EXPECT_TRUE(appendFuture.get().has_value());

    auto sealFuture = client->asyncCall(models::SealAppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(content.size()));
    auto sealOutcome = sealFuture.get();
    if (!sealOutcome.has_value()) {
        auto& error = sealOutcome.error();
        EXPECT_EQ("OperationNotSupported", error.getCode());
    }
}

TEST_F(AsyncObjectAppendTest, SealAppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::SealAppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
