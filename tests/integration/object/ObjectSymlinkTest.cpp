#include <gtest/gtest.h>

#include "../Config.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {

class ObjectSymlinkTest : public ::testing::Test {
  protected:
    ObjectSymlinkTest() {}

    ~ObjectSymlinkTest() override {}

    static void SetUpTestCase() {
        auto client = Config::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.isSuccess());
    }

    static void TearDownTestCase() {
        Config::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string ObjectSymlinkTest::bucketName_ = "";

// PutSymlink and GetSymlink Tests
TEST_F(ObjectSymlinkTest, Symlink_Normal) {
    auto client = Config::GetDefaultClient();
    std::string targetKey = "test-target-object";
    std::string symlinkKey = "test-symlink";
    std::string content = "Target content for symlink";

    // Put target object
    auto body = RequestBody::FromString(content);

    auto putOutcome =
            client->putObject(models::PutObjectRequest().setBucket(bucketName_).setKey(targetKey).setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Create symlink
    auto putSymlinkOutcome = client->putSymlink(
            models::PutSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey).setSymlinkTarget(targetKey));
    EXPECT_TRUE(putSymlinkOutcome.isSuccess());

    // Get symlink
    auto getSymlinkOutcome = client->getSymlink(models::GetSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey));
    EXPECT_TRUE(getSymlinkOutcome.isSuccess());
    auto& result = getSymlinkOutcome.getResult();
    EXPECT_EQ(targetKey, result.getSymlinkTarget());
}

TEST_F(ObjectSymlinkTest, PutSymlink_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->putSymlink(
            models::PutSymlinkRequest().setBucket(bucketName_).setKey("symlink-key").setSymlinkTarget("target-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

TEST_F(ObjectSymlinkTest, GetSymlink_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->getSymlink(models::GetSymlinkRequest().setBucket(bucketName_).setKey("symlink-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace oss2
} // namespace alibabacloud
