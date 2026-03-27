#include <gtest/gtest.h>

#include "../Config.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {

class ObjectBasicTest : public ::testing::Test {
  protected:
    ObjectBasicTest() {}

    ~ObjectBasicTest() override {}

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

std::string ObjectBasicTest::bucketName_ = "";

// PutObject Tests
TEST_F(ObjectBasicTest, PutObject_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-put-object";
    std::string content = "Hello, OSS!";
    auto body = RequestBody::FromString(content);

    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(outcome.isSuccess());
}

TEST_F(ObjectBasicTest, PutObject_WithMetadata) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-put-object-metadata";
    std::string content = "Hello with metadata!";
    auto body = RequestBody::FromString(content);


    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body)
            .setObjectAcl("private")
            .setStorageClass("Standard"));
    EXPECT_TRUE(outcome.isSuccess());
}

TEST_F(ObjectBasicTest, PutObject_Fail) {
    auto client = Config::GetInvalidClient();
    std::string key = "test-put-object-fail";
    std::string content = "Hello!";
    auto body = RequestBody::FromString(content);


    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// GetObject Tests
TEST_F(ObjectBasicTest, GetObject_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-get-object";
    std::string content = "Hello, GetObject!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Get object
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, GetObject_NotFound) {
    auto client = Config::GetDefaultClient();
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey("non-existent-object"));
    EXPECT_FALSE(outcome.isSuccess());
}

TEST_F(ObjectBasicTest, GetObject_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// CopyObject Tests
TEST_F(ObjectBasicTest, CopyObject_Normal) {
    auto client = Config::GetDefaultClient();
    std::string sourceKey = "test-copy-source";
    std::string destKey = "test-copy-dest";
    std::string content = "Copy me!";

    // Put source object
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(sourceKey)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Copy object
    auto outcome = client->copyObject(
        models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey(destKey)
            .setSourceBucket(bucketName_)
            .setSourceKey(sourceKey));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, CopyObject_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->copyObject(
        models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey("dest-key")
            .setSourceBucket(bucketName_)
            .setSourceKey("source-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// DeleteObject Tests
TEST_F(ObjectBasicTest, DeleteObject_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-delete-object";
    std::string content = "Delete me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Delete object
    auto outcome = client->deleteObject(
        models::DeleteObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.isSuccess());
}

TEST_F(ObjectBasicTest, DeleteObject_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->deleteObject(
        models::DeleteObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// HeadObject Tests
TEST_F(ObjectBasicTest, HeadObject_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-head-object";
    std::string content = "Head me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Head object
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, HeadObject_NotFound) {
    auto client = Config::GetDefaultClient();
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey("non-existent-object"));
    EXPECT_FALSE(outcome.isSuccess());
}

TEST_F(ObjectBasicTest, HeadObject_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// GetObjectMeta Tests
TEST_F(ObjectBasicTest, GetObjectMeta_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-meta-object";
    std::string content = "Meta me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Get object meta
    auto outcome = client->getObjectMeta(
        models::GetObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, GetObjectMeta_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->getObjectMeta(
        models::GetObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace oss2
} // namespace alibabacloud
