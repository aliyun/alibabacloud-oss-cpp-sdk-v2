#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncBucketObjectsTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());

        for (int i = 0; i < 5; i++) {
            auto key = "test-object-" + std::to_string(i);
            auto putFuture = client->asyncCall(models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString("content-" + std::to_string(i))));
            EXPECT_TRUE(putFuture.get().has_value());
        }
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncBucketObjectsTest::bucketName_ = "";

TEST_F(AsyncBucketObjectsTest, ListObjects_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::ListObjectsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(AsyncBucketObjectsTest, ListObjects_WithMaxKeys) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::ListObjectsRequest().setBucket(bucketName_).setMaxKeys(2));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_LE(outcome.value().getContents().size(), 2);
}

TEST_F(AsyncBucketObjectsTest, ListObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::ListObjectsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncBucketObjectsTest, ListObjectsV2_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::ListObjectsV2Request().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(AsyncBucketObjectsTest, ListObjectsV2_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::ListObjectsV2Request().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncBucketObjectsTest, DeleteMultipleObjects_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    std::vector<models::ObjectIdentifier> objects;
    for (int i = 10; i < 13; i++) {
        auto key = "delete-me-" + std::to_string(i);
        auto putFuture = client->asyncCall(models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::fromString("content-" + std::to_string(i))));
        EXPECT_TRUE(putFuture.get().has_value());

        models::ObjectIdentifier obj;
        obj.setKey(key);
        objects.push_back(obj);
    }

    models::Delete deleteReq;
    deleteReq.setObjects(objects);

    auto future = client->asyncCall(models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(3, outcome.value().getDeletedObjects().size());
}

TEST_F(AsyncBucketObjectsTest, DeleteMultipleObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    models::Delete deleteReq;
    std::vector<models::ObjectIdentifier> objects;
    models::ObjectIdentifier obj;
    obj.setKey("test-key");
    objects.push_back(obj);
    deleteReq.setObjects(objects);

    auto future = client->asyncCall(models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
