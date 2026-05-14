#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncBucketStatTest : public ::testing::Test {
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

std::string AsyncBucketStatTest::bucketName_ = "";

TEST_F(AsyncBucketStatTest, GetBucketStat_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::GetBucketStatRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_TRUE(result.hasBucketStat());
    auto& stat = result.getBucketStat();
    EXPECT_EQ(0, stat.objectCount.value_or(-1));
    EXPECT_EQ(0, stat.storage.value_or(-1));
}

TEST_F(AsyncBucketStatTest, GetBucketStat_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetBucketStatRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketStat", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
