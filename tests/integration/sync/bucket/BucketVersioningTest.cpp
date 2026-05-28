#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class BucketVersioningTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanVersioningBucket(bucketName_);
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string BucketVersioningTest::bucketName_ = "";

TEST_F(BucketVersioningTest, PutAndGetBucketVersioning_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto putOutcome = client->putBucketVersioning(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    EXPECT_TRUE(putOutcome.has_value());
    EXPECT_EQ(200, putOutcome.value().getStatusCode());

    Config::WaitForCacheExpire(2);

    auto getOutcome = client->getBucketVersioning(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    EXPECT_TRUE(getOutcome.has_value());
    EXPECT_TRUE(getOutcome.value().hasVersioningConfiguration());
    EXPECT_EQ("Enabled", getOutcome.value().getVersioningConfiguration().status.value());
}

TEST_F(BucketVersioningTest, PutBucketVersioning_Suspended) {
    auto client = ClientHelper::GetDefaultClient();

    auto putOutcome = client->putBucketVersioning(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Suspended")));
    EXPECT_TRUE(putOutcome.has_value());

    Config::WaitForCacheExpire(2);

    auto getOutcome = client->getBucketVersioning(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    EXPECT_TRUE(getOutcome.has_value());
    EXPECT_TRUE(getOutcome.value().hasVersioningConfiguration());
    EXPECT_EQ("Suspended", getOutcome.value().getVersioningConfiguration().status.value());
}

TEST_F(BucketVersioningTest, ListObjectVersions_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    // Enable versioning
    auto putVerOutcome = client->putBucketVersioning(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    EXPECT_TRUE(putVerOutcome.has_value());

    Config::WaitForCacheExpire(2);

    // Upload an object to create a version
    std::string key = "test-versioning-object";
    auto putOutcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString("version1")));
    EXPECT_TRUE(putOutcome.has_value());

    // Upload again to create a second version
    auto putOutcome2 = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString("version2")));
    EXPECT_TRUE(putOutcome2.has_value());

    // List versions
    auto listOutcome = client->listObjectVersions(
            models::ListObjectVersionsRequest()
                    .setBucket(bucketName_)
                    .setPrefix(key));
    EXPECT_TRUE(listOutcome.has_value());
    auto& result = listOutcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getVersions().size(), 2u);

    // Verify the versions are for our key
    for (auto& ver : result.getVersions()) {
        EXPECT_EQ(key, ver.key);
        EXPECT_FALSE(ver.versionId.empty());
    }
}

TEST_F(BucketVersioningTest, ListObjectVersions_WithDelimiter) {
    auto client = ClientHelper::GetDefaultClient();

    // Upload objects with prefix structure
    auto putOutcome1 = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("versioning-dir/obj1")
                    .setBody(RequestBody::fromString("content1")));
    EXPECT_TRUE(putOutcome1.has_value());

    auto putOutcome2 = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("versioning-dir/obj2")
                    .setBody(RequestBody::fromString("content2")));
    EXPECT_TRUE(putOutcome2.has_value());

    // List with delimiter
    auto listOutcome = client->listObjectVersions(
            models::ListObjectVersionsRequest()
                    .setBucket(bucketName_)
                    .setDelimiter("/")
                    .setPrefix("versioning-dir/"));
    EXPECT_TRUE(listOutcome.has_value());
    auto& result = listOutcome.value();
    EXPECT_GE(result.getVersions().size(), 2u);
}

TEST_F(BucketVersioningTest, ListObjectVersions_WithMaxKeys) {
    auto client = ClientHelper::GetDefaultClient();

    auto listOutcome = client->listObjectVersions(
            models::ListObjectVersionsRequest()
                    .setBucket(bucketName_)
                    .setMaxKeys(1));
    EXPECT_TRUE(listOutcome.has_value());
    auto& result = listOutcome.value();
    EXPECT_LE(result.getVersions().size() + result.getDeleteMarkers().size(), 1u);
}

TEST_F(BucketVersioningTest, PutBucketVersioning_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->putBucketVersioning(
            models::PutBucketVersioningRequest()
                    .setBucket(bucketName_)
                    .setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled")));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("PutBucketVersioning", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
}

TEST_F(BucketVersioningTest, GetBucketVersioning_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getBucketVersioning(
            models::GetBucketVersioningRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketVersioning", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

TEST_F(BucketVersioningTest, ListObjectVersions_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->listObjectVersions(
            models::ListObjectVersionsRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("ListObjectVersions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
