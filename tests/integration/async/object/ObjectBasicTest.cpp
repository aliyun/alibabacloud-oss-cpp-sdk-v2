#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectBasicTest : public ::testing::Test {
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

std::string AsyncObjectBasicTest::bucketName_ = "";

TEST_F(AsyncObjectBasicTest, PutObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-put-object";
    std::string content = "Hello, OSS!";

    auto future = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString(content)));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST_F(AsyncObjectBasicTest, PutObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey("test-key").setBody(RequestBody::FromString("hi")));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectBasicTest, GetObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-object";
    std::string content = "Hello, GetObject!";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(AsyncObjectBasicTest, GetObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetObjectRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectBasicTest, GetObject_WithSinkFactory) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-object-sinkfactory";
    std::string content = "Hello, Async SinkFactory!";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = false;

    auto future = client->asyncCall(models::GetObjectRequest().setBucket(bucketName_).setKey(key).setSinkFactory(factory));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content, userStream->str());
}

TEST_F(AsyncObjectBasicTest, CopyObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string sourceKey = "test-copy-source";
    std::string destKey = "test-copy-dest";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(sourceKey).setBody(RequestBody::FromString("Copy me!")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::CopyObjectRequest().setBucket(bucketName_).setKey(destKey).setSourceBucket(bucketName_).setSourceKey(sourceKey));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value().getETag().empty());
}

TEST_F(AsyncObjectBasicTest, CopyObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::CopyObjectRequest().setBucket(bucketName_).setKey("dest").setSourceBucket(bucketName_).setSourceKey("src"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectBasicTest, DeleteObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-delete-object";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString("Delete me!")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::DeleteObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST_F(AsyncObjectBasicTest, DeleteObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::DeleteObjectRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectBasicTest, HeadObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-head-object";
    std::string content = "Head me!";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::HeadObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), outcome.value().getContentLength());
    EXPECT_FALSE(outcome.value().getETag().empty());
}

TEST_F(AsyncObjectBasicTest, HeadObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::HeadObjectRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectBasicTest, GetObjectMeta_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-meta-object";
    std::string content = "Meta me!";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(models::GetObjectMetaRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), outcome.value().getContentLength());
    EXPECT_FALSE(outcome.value().getETag().empty());
}

TEST_F(AsyncObjectBasicTest, GetObjectMeta_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetObjectMetaRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
