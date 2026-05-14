#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class BucketBasicTest : public ::testing::Test {
  protected:
    BucketBasicTest() {}

    ~BucketBasicTest() override {}

    // Sets up the stuff shared by all tests in this test case.
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
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

std::string BucketBasicTest::bucketName_ = "";

// GetBucketInfo
TEST_F(BucketBasicTest, GetBucketInfo_Noraml) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->getBucketInfo(models::GetBucketInfoRequest().setBucket(bucketName_));
    EXPECT_EQ(true, outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(true, result.hasBucketInfo());
    auto& info = result.getBucketInfo();
    EXPECT_EQ(bucketName_, info.name);
    EXPECT_EQ("Standard", info.storageClass);
}

TEST_F(BucketBasicTest, GetBucketInfo_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getBucketInfo(models::GetBucketInfoRequest().setBucket(bucketName_));
    EXPECT_EQ(false, outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketInfo", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

// BucketLocation
TEST_F(BucketBasicTest, BucketLocation_Noraml) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->getBucketLocation(models::GetBucketLocationRequest().setBucket(bucketName_));
    EXPECT_EQ(true, outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ("oss-" + Config::Region, result.getLocationConstraint());
}

TEST_F(BucketBasicTest, BucketLocation_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getBucketLocation(models::GetBucketLocationRequest().setBucket(bucketName_));
    EXPECT_EQ(false, outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketLocation", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}


} // namespace sync
} // namespace oss2
} // namespace alibabacloud
