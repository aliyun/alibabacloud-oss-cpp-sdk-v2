#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectSymlinkTest : public ::testing::Test {
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

std::string AsyncObjectSymlinkTest::bucketName_ = "";

TEST_F(AsyncObjectSymlinkTest, Symlink_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string targetKey = "test-target-object";
    std::string symlinkKey = "test-symlink";

    auto putFuture = client->callAsync<PutObjectOutcome>(
        &OSSAsyncClient::putObjectAsync,
        models::PutObjectRequest().setBucket(bucketName_).setKey(targetKey).setBody(RequestBody::FromString("target content")));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto putSymFuture = client->callAsync<PutSymlinkOutcome>(
        &OSSAsyncClient::putSymlinkAsync,
        models::PutSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey).setSymlinkTarget(targetKey));
    EXPECT_TRUE(putSymFuture.get().isSuccess());

    auto getSymFuture = client->callAsync<GetSymlinkOutcome>(
        &OSSAsyncClient::getSymlinkAsync, models::GetSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey));
    auto outcome = getSymFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(targetKey, outcome.getResult().getSymlinkTarget());
}

TEST_F(AsyncObjectSymlinkTest, PutSymlink_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<PutSymlinkOutcome>(
        &OSSAsyncClient::putSymlinkAsync,
        models::PutSymlinkRequest().setBucket(bucketName_).setKey("symlink-key").setSymlinkTarget("target-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectSymlinkTest, GetSymlink_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<GetSymlinkOutcome>(
        &OSSAsyncClient::getSymlinkAsync, models::GetSymlinkRequest().setBucket(bucketName_).setKey("symlink-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
