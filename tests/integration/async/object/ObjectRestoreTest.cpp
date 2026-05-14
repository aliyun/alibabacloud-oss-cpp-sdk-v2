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

std::string AsyncObjectRestoreTest::bucketName_ = "";

TEST_F(AsyncObjectRestoreTest, RestoreObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-restore-object";

    auto putFuture = client->callAsync<PutObjectOutcome>(
        &OSSAsyncClient::putObjectAsync,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key)
            .setBody(RequestBody::FromString("Archive content")).setStorageClass("Archive"));
    EXPECT_TRUE(putFuture.get().isSuccess());

    models::RestoreRequest restoreReq;
    restoreReq.setDays(1);
    models::JobParameters jobParams;
    jobParams.setTier("Standard");
    restoreReq.setJobParameters(jobParams);

    auto future = client->callAsync<RestoreObjectOutcome>(
        &OSSAsyncClient::restoreObjectAsync,
        models::RestoreObjectRequest().setBucket(bucketName_).setKey(key).setRestoreRequest(restoreReq));
    future.get();
}

TEST_F(AsyncObjectRestoreTest, RestoreObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    models::RestoreRequest restoreReq;
    auto future = client->callAsync<RestoreObjectOutcome>(
        &OSSAsyncClient::restoreObjectAsync,
        models::RestoreObjectRequest().setBucket(bucketName_).setKey("test-key").setRestoreRequest(restoreReq));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectRestoreTest, CleanRestoredObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-clean-restored-object";

    auto putFuture = client->callAsync<PutObjectOutcome>(
        &OSSAsyncClient::putObjectAsync,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key)
            .setBody(RequestBody::FromString("Content to clean")).setStorageClass("Archive"));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto future = client->callAsync<CleanRestoredObjectOutcome>(
        &OSSAsyncClient::cleanRestoredObjectAsync,
        models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey(key));
    future.get();
}

TEST_F(AsyncObjectRestoreTest, CleanRestoredObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<CleanRestoredObjectOutcome>(
        &OSSAsyncClient::cleanRestoredObjectAsync,
        models::CleanRestoredObjectRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
