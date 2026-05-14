#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class BucketStatTest : public ::testing::Test {
  protected:
    BucketStatTest() {}

    ~BucketStatTest() override {}

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

std::string BucketStatTest::bucketName_ = "";

TEST_F(BucketStatTest, GetBucketStat_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto outcome = client->getBucketStat(models::GetBucketStatRequest().setBucket(bucketName_));
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_TRUE(result.hasBucketStat());
    auto& stat = result.getBucketStat();
    // New bucket should have 0 objects and 0 storage
    EXPECT_EQ(0, stat.objectCount.value_or(-1));
    EXPECT_EQ(0, stat.storage.value_or(-1));
}

TEST_F(BucketStatTest, GetBucketStat_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome = client->getBucketStat(models::GetBucketStatRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketStat", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
