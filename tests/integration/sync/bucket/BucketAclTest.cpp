
#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class BucketAclTest : public ::testing::Test {
  protected:
    BucketAclTest() {}

    ~BucketAclTest() override {}

    // Sets up the stuff shared by all tests in this test case.
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.isSuccess());
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

    // Sets up the test fixture.
    void SetUp() override {}

    // Tears down the test fixture.
    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string BucketAclTest::bucketName_ = "";

TEST_F(BucketAclTest, BucketAcl_Noraml) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->putBucketAcl(models::PutBucketAclRequest().setBucket(bucketName_).setAcl("private"));
    EXPECT_EQ(true, outcome.isSuccess());

    Config::WaitForCacheExpire(2);
    auto getOutcome = client->getBucketAcl(models::GetBucketAclRequest().setBucket(bucketName_));
    EXPECT_EQ(true, getOutcome.isSuccess());
    auto& result = getOutcome.getResult();
    EXPECT_EQ("private", result.getAccessControlPolicy().accessControlList.value().grant);
}

TEST_F(BucketAclTest, PutBucketAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome =
            client->putBucketAcl(models::PutBucketAclRequest().setBucket(bucketName_).setAcl("private"));
    EXPECT_EQ(false, outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("PutBucketAcl", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
}

TEST_F(BucketAclTest, GetBucketAcl_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome =
            client->getBucketAcl(models::GetBucketAclRequest().setBucket(bucketName_));
    EXPECT_EQ(false, outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketAcl", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud