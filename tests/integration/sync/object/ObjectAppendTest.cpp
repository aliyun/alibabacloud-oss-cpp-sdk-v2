#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectAppendTest : public ::testing::Test {
  protected:
    ObjectAppendTest() {}

    ~ObjectAppendTest() override {}

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

std::string ObjectAppendTest::bucketName_ = "";

// AppendObject Tests
TEST_F(ObjectAppendTest, AppendObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-append-object";
    std::string content1 = "Hello, ";
    std::string content2 = "World!";

    // First append (position must be 0)
    auto body1 = RequestBody::FromString(content1);
    auto outcome1 = client->appendObject(
            models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(body1));
    EXPECT_TRUE(outcome1.has_value());
    auto& result1 = outcome1.value();
    EXPECT_EQ(content1.size(), result1.getNextAppendPosition());

    // Second append
    auto body2 = RequestBody::FromString(content2);
    auto outcome2 = client->appendObject(models::AppendObjectRequest()
                                                 .setBucket(bucketName_)
                                                 .setKey(key)
                                                 .setPosition(result1.getNextAppendPosition())
                                                 .setBody(body2));
    EXPECT_TRUE(outcome2.has_value());
    auto& result2 = outcome2.value();
    EXPECT_EQ(content1.size() + content2.size(), result2.getNextAppendPosition());
}

TEST_F(ObjectAppendTest, AppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    std::string content = "Test content";
    auto body = RequestBody::FromString(content);

    auto outcome = client->appendObject(
            models::AppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0).setBody(body));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// SealAppendObject Tests
TEST_F(ObjectAppendTest, SealAppendObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-seal-append-object";
    std::string content = "Content to seal";

    // Create appendable object
    auto body = RequestBody::FromString(content);
    auto appendOutcome = client->appendObject(
            models::AppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(0).setBody(body));
    EXPECT_TRUE(appendOutcome.has_value());

    // Seal the appendable object
    auto sealOutcome = client->sealAppendObject(
            models::SealAppendObjectRequest().setBucket(bucketName_).setKey(key).setPosition(content.size()));
    if (!sealOutcome.has_value()) {
        auto& error = sealOutcome.error();
        EXPECT_EQ("OperationNotSupported", error.getCode());
        EXPECT_EQ("SealAppendable is not supported.", error.getMessage());
    } else {
        EXPECT_TRUE(sealOutcome.has_value());
    }
}

TEST_F(ObjectAppendTest, SealAppendObject_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->sealAppendObject(
            models::SealAppendObjectRequest().setBucket(bucketName_).setKey("test-key").setPosition(0));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
