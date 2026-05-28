#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncBucketVersioningTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanVersioningBucket(bucketName_);
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncBucketVersioningTest::bucketName_ = "";

TEST_F(AsyncBucketVersioningTest, PutAndGetBucketVersioning_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto putFuture = client->asyncCall(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    auto putOutcome = putFuture.get();
    EXPECT_TRUE(putOutcome.has_value());
    EXPECT_EQ(200, putOutcome.value().getStatusCode());

    Config::WaitForCacheExpire(2);

    auto getFuture = client->asyncCall(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    auto getOutcome = getFuture.get();
    EXPECT_TRUE(getOutcome.has_value());
    EXPECT_TRUE(getOutcome.value().hasVersioningConfiguration());
    EXPECT_EQ("Enabled", getOutcome.value().getVersioningConfiguration().status.value());
}

TEST_F(AsyncBucketVersioningTest, PutBucketVersioning_Suspended) {
    auto client = ClientHelper::GetDefaultClient();

    auto putFuture = client->asyncCall(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Suspended")));
    auto putOutcome = putFuture.get();
    EXPECT_TRUE(putOutcome.has_value());

    Config::WaitForCacheExpire(2);

    auto getFuture = client->asyncCall(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    auto getOutcome = getFuture.get();
    EXPECT_TRUE(getOutcome.has_value());
    EXPECT_TRUE(getOutcome.value().hasVersioningConfiguration());
    EXPECT_EQ("Suspended", getOutcome.value().getVersioningConfiguration().status.value());
}

TEST_F(AsyncBucketVersioningTest, ListObjectVersions_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    // Enable versioning
    auto putVerFuture = client->asyncCall(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    EXPECT_TRUE(putVerFuture.get().has_value());

    Config::WaitForCacheExpire(2);

    // Upload an object to create a version
    std::string key = "async-test-versioning-object";
    auto putFuture1 = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString("version1")));
    EXPECT_TRUE(putFuture1.get().has_value());

    // Upload again to create a second version
    auto putFuture2 = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString("version2")));
    EXPECT_TRUE(putFuture2.get().has_value());

    // List versions
    auto listFuture = client->asyncCall(
            models::ListObjectVersionsRequest()
                    .setBucket(bucketName_)
                    .setPrefix(key));
    auto listOutcome = listFuture.get();
    EXPECT_TRUE(listOutcome.has_value());
    auto& result = listOutcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getVersions().size(), 2u);

    for (auto& ver : result.getVersions()) {
        EXPECT_EQ(key, ver.key);
        EXPECT_FALSE(ver.versionId.empty());
    }
}

TEST_F(AsyncBucketVersioningTest, PutBucketVersioning_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("PutBucketVersioning", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
}

TEST_F(AsyncBucketVersioningTest, GetBucketVersioning_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketVersioning", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

TEST_F(AsyncBucketVersioningTest, ListObjectVersions_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(
            models::ListObjectVersionsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("ListObjectVersions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
