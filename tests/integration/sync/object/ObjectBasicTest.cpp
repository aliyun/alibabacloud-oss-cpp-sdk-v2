#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectBasicTest : public ::testing::Test {
  protected:
    ObjectBasicTest() {}

    ~ObjectBasicTest() override {}

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

std::string ObjectBasicTest::bucketName_ = "";

// PutObject Tests
TEST_F(ObjectBasicTest, PutObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-put-object";
    std::string content = "Hello, OSS!";
    auto body = RequestBody::FromString(content);

    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(outcome.has_value());
}

TEST_F(ObjectBasicTest, PutObject_WithMetadata) {
    auto client = ClientHelper::GetDefaultClient();
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
    EXPECT_TRUE(outcome.has_value());
}

TEST_F(ObjectBasicTest, PutObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    std::string key = "test-put-object-fail";
    std::string content = "Hello!";
    auto body = RequestBody::FromString(content);


    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// GetObject Tests
TEST_F(ObjectBasicTest, GetObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-object";
    std::string content = "Hello, GetObject!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    // Get object
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, GetObject_NotFound) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey("non-existent-object"));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(ObjectBasicTest, GetObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

TEST_F(ObjectBasicTest, GetObject_WithSinkFactory) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-object-sinkfactory";
    std::string content = "Hello, SinkFactory!";

    auto body = RequestBody::FromString(content);
    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = false;

    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setSinkFactory(factory));
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content, userStream->str());
}

TEST_F(ObjectBasicTest, GetObject_WithSinkFactory_OneShot) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-object-sinkfactory-oneshot";
    std::string content = "OneShot content!";

    auto body = RequestBody::FromString(content);
    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = true;

    auto outcome = client->getObject(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setSinkFactory(factory));
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content, userStream->str());
}

// CopyObject Tests
TEST_F(ObjectBasicTest, CopyObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
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
    EXPECT_TRUE(putOutcome.has_value());

    // Copy object
    auto outcome = client->copyObject(
        models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey(destKey)
            .setSourceBucket(bucketName_)
            .setSourceKey(sourceKey));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, CopyObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->copyObject(
        models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey("dest-key")
            .setSourceBucket(bucketName_)
            .setSourceKey("source-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// DeleteObject Tests
TEST_F(ObjectBasicTest, DeleteObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-delete-object";
    std::string content = "Delete me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    // Delete object
    auto outcome = client->deleteObject(
        models::DeleteObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.has_value());
}

TEST_F(ObjectBasicTest, DeleteObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->deleteObject(
        models::DeleteObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// HeadObject Tests
TEST_F(ObjectBasicTest, HeadObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-head-object";
    std::string content = "Head me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    // Head object
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, HeadObject_NotFound) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey("non-existent-object"));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(ObjectBasicTest, HeadObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// GetObjectMeta Tests
TEST_F(ObjectBasicTest, GetObjectMeta_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-meta-object";
    std::string content = "Meta me!";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    // Get object meta
    auto outcome = client->getObjectMeta(
        models::GetObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectBasicTest, GetObjectMeta_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getObjectMeta(
        models::GetObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// --- CRC64 Upload Check Integration Tests ---

TEST_F(ObjectBasicTest, PutObject_CRC64CheckUpload) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-put-object-crc64";
    std::string content = "Hello, OSS CRC64 upload check!";
    auto body = RequestBody::FromString(content);

    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(body));
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value().getHashCrc64ecma().empty());
}

TEST_F(ObjectBasicTest, AppendObject_CRC64CheckUpload) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-append-object-crc64";
    std::string content1 = "First part ";
    std::string content2 = "Second part";

    auto outcome1 = client->appendObject(
        models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(0)
            .setInitHashCRC64(0)
            .setBody(RequestBody::FromString(content1)));
    EXPECT_TRUE(outcome1.has_value());
    EXPECT_FALSE(outcome1.value().getHashCrc64ecma().empty());

    auto outcome2 = client->appendObject(
        models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(outcome1.value().getNextAppendPosition())
            .setInitHashCRC64(outcome1.value().getHashCrc64ecmaAsUint64())
            .setBody(RequestBody::FromString(content2)));
    EXPECT_TRUE(outcome2.has_value());
    EXPECT_FALSE(outcome2.value().getHashCrc64ecma().empty());
}

TEST_F(ObjectBasicTest, AppendObject_CRC64CheckUpload_NoInit) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-append-object-crc64-noinit";
    std::string content = "Append without init CRC";

    auto outcome = client->appendObject(
        models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(0)
            .setBody(RequestBody::FromString(content)));
    EXPECT_TRUE(outcome.has_value());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
