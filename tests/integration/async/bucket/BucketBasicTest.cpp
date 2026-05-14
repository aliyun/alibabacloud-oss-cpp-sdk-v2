#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncBucketBasicTest : public ::testing::Test {
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

std::string AsyncBucketBasicTest::bucketName_ = "";

TEST_F(AsyncBucketBasicTest, GetBucketInfo_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::GetBucketInfoRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_TRUE(result.hasBucketInfo());
    auto& info = result.getBucketInfo();
    EXPECT_EQ(bucketName_, info.name);
    EXPECT_EQ("Standard", info.storageClass);
}

TEST_F(AsyncBucketBasicTest, GetBucketInfo_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetBucketInfoRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketInfo", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

TEST_F(AsyncBucketBasicTest, BucketLocation_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto future = client->asyncCall(models::GetBucketLocationRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ("oss-" + Config::Region, result.getLocationConstraint());
}

TEST_F(AsyncBucketBasicTest, BucketLocation_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::GetBucketLocationRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("GetBucketLocation", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
