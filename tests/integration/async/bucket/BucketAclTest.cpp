#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncBucketAclTest : public ::testing::Test {
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

std::string AsyncBucketAclTest::bucketName_ = "";

TEST_F(AsyncBucketAclTest, BucketAcl_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::PutBucketAclRequest().setBucket(bucketName_).setAcl("private"));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());

    Config::WaitForCacheExpire(2);
    auto future2 = client->asyncCall(models::GetBucketAclRequest().setBucket(bucketName_));
    auto outcome2 = future2.get();
    EXPECT_TRUE(outcome2.has_value());
    auto& result = outcome2.value();
    EXPECT_EQ("private", result.getAccessControlPolicy().accessControlList.value().grant);
}

TEST_F(AsyncBucketAclTest, PutBucketAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::PutBucketAclRequest().setBucket(bucketName_).setAcl("private"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("PutBucketAcl", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
}

TEST_F(AsyncBucketAclTest, GetBucketAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetBucketAclRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketAcl", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
