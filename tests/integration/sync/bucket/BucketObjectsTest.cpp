#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class BucketObjectsTest : public ::testing::Test {
  protected:
    BucketObjectsTest() {}

    ~BucketObjectsTest() override {}

    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());

        // Upload some test objects
        for (int i = 0; i < 5; i++) {
            auto key = "test-object-" + std::to_string(i);
            auto putOutcome =
                    client->putObject(models::PutObjectRequest()
                                              .setBucket(bucketName_)
                                              .setKey(key)
                                              .setBody(RequestBody::fromString("content-" + std::to_string(i))));
            EXPECT_TRUE(putOutcome.has_value());
        }
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string BucketObjectsTest::bucketName_ = "";

// ListObjects Tests
TEST_F(BucketObjectsTest, ListObjects_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->listObjects(models::ListObjectsRequest().setBucket(bucketName_));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(BucketObjectsTest, ListObjects_WithPrefix) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->listObjects(models::ListObjectsRequest().setBucket(bucketName_).setPrefix("test-object-"));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(BucketObjectsTest, ListObjects_WithMaxKeys) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->listObjects(models::ListObjectsRequest().setBucket(bucketName_).setMaxKeys(2));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_LE(result.getContents().size(), 2);
}

TEST_F(BucketObjectsTest, ListObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->listObjects(models::ListObjectsRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// ListObjectsV2 Tests
TEST_F(BucketObjectsTest, ListObjectsV2_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->listObjectsV2(models::ListObjectsV2Request().setBucket(bucketName_));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(BucketObjectsTest, ListObjectsV2_WithPrefix) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome =
            client->listObjectsV2(models::ListObjectsV2Request().setBucket(bucketName_).setPrefix("test-object-"));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(BucketObjectsTest, ListObjectsV2_WithMaxKeys) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->listObjectsV2(models::ListObjectsV2Request().setBucket(bucketName_).setMaxKeys(2));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_LE(result.getContents().size(), 2);
}

TEST_F(BucketObjectsTest, ListObjectsV2_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->listObjectsV2(models::ListObjectsV2Request().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// DeleteMultipleObjects Tests
TEST_F(BucketObjectsTest, DeleteMultipleObjects_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    // Create objects to delete
    std::vector<models::ObjectIdentifier> objects;
    for (int i = 10; i < 13; i++) {
        auto key = "delete-me-" + std::to_string(i);
        auto putOutcome = client->putObject(models::PutObjectRequest()
                                                    .setBucket(bucketName_)
                                                    .setKey(key)
                                                    .setBody(RequestBody::fromString("content-" + std::to_string(i))));
        EXPECT_TRUE(putOutcome.has_value());

        models::ObjectIdentifier obj;
        obj.setKey(key);
        objects.push_back(obj);
    }

    models::Delete deleteReq;
    deleteReq.setObjects(objects);

    auto outcome = client->deleteMultipleObjects(
            models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(3, result.getDeletedObjects().size());
}

TEST_F(BucketObjectsTest, DeleteMultipleObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    models::Delete deleteReq;
    std::vector<models::ObjectIdentifier> objects;
    models::ObjectIdentifier obj;
    obj.setKey("test-key");
    objects.push_back(obj);
    deleteReq.setObjects(objects);

    auto outcome = client->deleteMultipleObjects(
            models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
