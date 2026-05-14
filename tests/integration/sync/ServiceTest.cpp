#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"


namespace alibabacloud {
namespace oss2 {
namespace sync {

TEST(ServiceTest, ListBuckets_normal) {
    auto client = ClientHelper::GetDefaultClient();

    // all
    auto request = models::ListBucketsRequest();
    auto outcome = client->listBuckets(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(24, result.getRequestId().size());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(false, result.getBuckets().empty());

    // oss-cn-hangzhou
    request.setMaxKeys(1);
    outcome = client->listBuckets(request);
    result = outcome.getResult();
    EXPECT_EQ(1, result.getBuckets().size());
}

TEST(ServiceTest, ListBuckets_fail) {
    auto client = ClientHelper::GetInvalidClient();

    auto request = models::ListBucketsRequest();
    auto outcome = client->listBuckets(request);
    EXPECT_FALSE(outcome.isSuccess());

    auto& error = outcome.getError();
    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListBuckets", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ(24, error.getRequestId().size());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
