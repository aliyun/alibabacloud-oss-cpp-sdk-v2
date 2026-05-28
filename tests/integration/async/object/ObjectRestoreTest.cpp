#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectRestoreTest : public ::testing::Test {
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

std::string AsyncObjectRestoreTest::bucketName_ = "";

TEST_F(AsyncObjectRestoreTest, RestoreObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-restore-object";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key)
            .setBody(RequestBody::fromString("Archive content")).setStorageClass("Archive"));
    EXPECT_TRUE(putFuture.get().has_value());

    models::RestoreRequest restoreReq;
    restoreReq.setDays(1);
    models::JobParameters jobParams;
    jobParams.setTier("Standard");
    restoreReq.setJobParameters(jobParams);

    auto future = client->asyncCall(models::RestoreObjectRequest().setBucket(bucketName_).setKey(key).setRestoreRequest(restoreReq));
    (void)future.get();
}

TEST_F(AsyncObjectRestoreTest, RestoreObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    models::RestoreRequest restoreReq;
    auto future = client->asyncCall(models::RestoreObjectRequest().setBucket(bucketName_).setKey("test-key").setRestoreRequest(restoreReq));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectRestoreTest, CleanRestoredObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-clean-restored-object";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key)
            .setBody(RequestBody::fromString("Content to clean")).setStorageClass("Archive"));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey(key));
    (void)future.get();
}

TEST_F(AsyncObjectRestoreTest, CleanRestoredObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
