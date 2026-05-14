#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectAclTest : public ::testing::Test {
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

std::string AsyncObjectAclTest::bucketName_ = "";

TEST_F(AsyncObjectAclTest, ObjectAcl_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-acl-object";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString("content")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto putAclFuture = client->asyncCall(models::PutObjectAclRequest().setBucket(bucketName_).setKey(key).setObjectAcl("private"));
    EXPECT_TRUE(putAclFuture.get().has_value());

    Config::WaitForCacheExpire(2);

    auto getAclFuture = client->asyncCall(models::GetObjectAclRequest().setBucket(bucketName_).setKey(key));
    auto outcome = getAclFuture.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_TRUE(result.hasAccessControlPolicy());
    auto& acl = result.getAccessControlPolicy();
    EXPECT_TRUE(acl.accessControlList.has_value());
    EXPECT_EQ("private", acl.accessControlList.value().grant.value_or(""));
}

TEST_F(AsyncObjectAclTest, PutObjectAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::PutObjectAclRequest().setBucket(bucketName_).setKey("test-key").setObjectAcl("private"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
    EXPECT_EQ("PutObjectAcl", outcome.error().getOpName());
}

TEST_F(AsyncObjectAclTest, GetObjectAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetObjectAclRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
    EXPECT_EQ("GetObjectAcl", outcome.error().getOpName());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
