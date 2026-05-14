#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectAclTest : public ::testing::Test {
  protected:
    ObjectAclTest() {}

    ~ObjectAclTest() override {}

    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.isSuccess());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string ObjectAclTest::bucketName_ = "";

// PutObjectAcl and GetObjectAcl Tests
TEST_F(ObjectAclTest, ObjectAcl_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-acl-object";
    std::string content = "ACL test content";

    // Put object first
    auto putOutcome = client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString("content")));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Set object ACL to public-read
    auto putAclOutcome = client->putObjectAcl(
            models::PutObjectAclRequest().setBucket(bucketName_).setKey(key).setObjectAcl("private"));
    EXPECT_TRUE(putAclOutcome.isSuccess());

    Config::WaitForCacheExpire(2);

    // Get object ACL
    auto getAclOutcome = client->getObjectAcl(models::GetObjectAclRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(getAclOutcome.isSuccess());
    auto& result = getAclOutcome.getResult();
    EXPECT_TRUE(result.hasAccessControlPolicy());
    auto& acl = result.getAccessControlPolicy();
    EXPECT_TRUE(acl.accessControlList.has_value());
    EXPECT_EQ("private", acl.accessControlList.value().grant.value_or(""));
}

TEST_F(ObjectAclTest, PutObjectAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->putObjectAcl(
            models::PutObjectAclRequest().setBucket(bucketName_).setKey("test-key").setObjectAcl("private"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("PutObjectAcl", error.getOpName());
}

TEST_F(ObjectAclTest, GetObjectAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getObjectAcl(models::GetObjectAclRequest().setBucket(bucketName_).setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetObjectAcl", error.getOpName());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
