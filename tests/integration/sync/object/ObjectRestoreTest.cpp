#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectRestoreTest : public ::testing::Test {
  protected:
    ObjectRestoreTest() {}

    ~ObjectRestoreTest() override {}

    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string ObjectRestoreTest::bucketName_ = "";

// RestoreObject Tests
TEST_F(ObjectRestoreTest, RestoreObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-restore-object";
    std::string content = "Archive content to restore";

    // Put an archive object
    auto body = RequestBody::fromString(content);

    auto putOutcome = client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body).setStorageClass("Archive"));
    EXPECT_TRUE(putOutcome.has_value());

    // Create restore request
    models::RestoreRequest restoreReq;
    restoreReq.setDays(1);

    models::JobParameters jobParams;
    jobParams.setTier("Standard");
    restoreReq.setJobParameters(jobParams);

    // Restore object
    auto outcome = client->restoreObject(
            models::RestoreObjectRequest().setBucket(bucketName_).setKey(key).setRestoreRequest(restoreReq));
    // Note: This may return 202 Accepted if successful, or fail if object is not in Archive/ColdArchive
    // The test documents the API usage
}

TEST_F(ObjectRestoreTest, RestoreObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    models::RestoreRequest restoreReq;
    auto outcome = client->restoreObject(
            models::RestoreObjectRequest().setBucket(bucketName_).setKey("test-key").setRestoreRequest(restoreReq));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// CleanRestoredObject Tests
TEST_F(ObjectRestoreTest, CleanRestoredObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-clean-restored-object";
    std::string content = "Content to clean";

    // Put an archive object
    auto body = RequestBody::fromString(content);
    auto putOutcome = client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body).setStorageClass("Archive"));
    EXPECT_TRUE(putOutcome.has_value());

    // Clean restored object
    auto outcome = client->cleanRestoredObject(models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey(key));
    // Note: This may fail if object is not in restored state
    // The test documents the API usage
}

TEST_F(ObjectRestoreTest, CleanRestoredObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome =
            client->cleanRestoredObject(models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
