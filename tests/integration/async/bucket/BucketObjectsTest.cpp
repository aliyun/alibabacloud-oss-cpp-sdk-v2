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
        auto future = client->callAsync<PutBucketOutcome>(&OSSAsyncClient::putBucketAsync,
                                                          models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().isSuccess());

        for (int i = 0; i < 5; i++) {
            auto key = "test-object-" + std::to_string(i);
            auto putFuture = client->callAsync<PutObjectOutcome>(
                &OSSAsyncClient::putObjectAsync,
                models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString("content-" + std::to_string(i))));
            EXPECT_TRUE(putFuture.get().isSuccess());
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
    auto future = client->callAsync<ListObjectsOutcome>(
        &OSSAsyncClient::listObjectsAsync, models::ListObjectsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(AsyncBucketObjectsTest, ListObjects_WithMaxKeys) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->callAsync<ListObjectsOutcome>(
        &OSSAsyncClient::listObjectsAsync, models::ListObjectsRequest().setBucket(bucketName_).setMaxKeys(2));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_LE(outcome.getResult().getContents().size(), 2);
}

TEST_F(AsyncBucketObjectsTest, ListObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<ListObjectsOutcome>(
        &OSSAsyncClient::listObjectsAsync, models::ListObjectsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncBucketObjectsTest, ListObjectsV2_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->callAsync<ListObjectsV2Outcome>(
        &OSSAsyncClient::listObjectsV2Async, models::ListObjectsV2Request().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(bucketName_, result.getName());
    EXPECT_GE(result.getContents().size(), 5);
}

TEST_F(AsyncBucketObjectsTest, ListObjectsV2_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<ListObjectsV2Outcome>(
        &OSSAsyncClient::listObjectsV2Async, models::ListObjectsV2Request().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncBucketObjectsTest, DeleteMultipleObjects_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    std::vector<models::ObjectIdentifier> objects;
    for (int i = 10; i < 13; i++) {
        auto key = "delete-me-" + std::to_string(i);
        auto putFuture = client->callAsync<PutObjectOutcome>(
            &OSSAsyncClient::putObjectAsync,
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString("content-" + std::to_string(i))));
        EXPECT_TRUE(putFuture.get().isSuccess());

        models::ObjectIdentifier obj;
        obj.setKey(key);
        objects.push_back(obj);
    }

    models::Delete deleteReq;
    deleteReq.setObjects(objects);

    auto future = client->callAsync<DeleteMultipleObjectsOutcome>(
        &OSSAsyncClient::deleteMultipleObjectsAsync,
        models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(3, outcome.getResult().getDeletedObjects().size());
}

TEST_F(AsyncBucketObjectsTest, DeleteMultipleObjects_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    models::Delete deleteReq;
    std::vector<models::ObjectIdentifier> objects;
    models::ObjectIdentifier obj;
    obj.setKey("test-key");
    objects.push_back(obj);
    deleteReq.setObjects(objects);

    auto future = client->callAsync<DeleteMultipleObjectsOutcome>(
        &OSSAsyncClient::deleteMultipleObjectsAsync,
        models::DeleteMultipleObjectsRequest().setBucket(bucketName_).setDelete(deleteReq));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
