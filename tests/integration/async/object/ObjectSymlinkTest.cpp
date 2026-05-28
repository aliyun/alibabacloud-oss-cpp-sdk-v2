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
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
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

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(targetKey).setBody(RequestBody::fromString("target content")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto putSymFuture = client->asyncCall(models::PutSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey).setSymlinkTarget(targetKey));
    EXPECT_TRUE(putSymFuture.get().has_value());

    auto getSymFuture = client->asyncCall(models::GetSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey));
    auto outcome = getSymFuture.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(targetKey, outcome.value().getSymlinkTarget());
}

TEST_F(AsyncObjectSymlinkTest, PutSymlink_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::PutSymlinkRequest().setBucket(bucketName_).setKey("symlink-key").setSymlinkTarget("target-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectSymlinkTest, GetSymlink_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetSymlinkRequest().setBucket(bucketName_).setKey("symlink-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
