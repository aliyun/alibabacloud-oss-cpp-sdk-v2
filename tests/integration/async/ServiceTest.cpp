#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

TEST(AsyncServiceTest, ListBuckets_normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto request = models::ListBucketsRequest();
    auto future = client->asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(24, result.getRequestId().size());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(false, result.getBuckets().empty());

    request.setMaxKeys(1);
    auto future2 = client->asyncCall(request);
    auto outcome2 = future2.get();
    auto& result2 = outcome2.getResult();
    EXPECT_EQ(1, result2.getBuckets().size());
}

TEST(AsyncServiceTest, ListBuckets_fail) {
    auto client = ClientHelper::GetInvalidClient();

    auto request = models::ListBucketsRequest();
    auto future = client->asyncCall(request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());

    auto& error = outcome.getError();
    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListBuckets", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ(24, error.getRequestId().size());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
