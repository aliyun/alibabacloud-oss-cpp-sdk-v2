#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

TEST(AsyncRegionTest, DescribeRegions_normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto request = models::DescribeRegionsRequest();
    auto future = client->callAsync<DescribeRegionsOutcome>(&OSSAsyncClient::describeRegionsAsync, request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(24, result.getRequestId().size());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(true, result.hasRegionInfoList());
    EXPECT_TRUE(result.getRegionInfoList().regionInfos.size() > 0);
    EXPECT_EQ("oss-ap-northeast-1", result.getRegionInfoList().regionInfos.at(0).region);

    request.setRegions("oss-cn-hangzhou");
    auto future2 = client->callAsync<DescribeRegionsOutcome>(&OSSAsyncClient::describeRegionsAsync, request);
    auto outcome2 = future2.get();
    auto& result2 = outcome2.getResult();
    EXPECT_EQ(1, result2.getRegionInfoList().regionInfos.size());
    EXPECT_EQ("oss-cn-hangzhou", result2.getRegionInfoList().regionInfos.at(0).region);
}

TEST(AsyncRegionTest, DescribeRegions_fail) {
    auto client = ClientHelper::GetInvalidClient();

    auto request = models::DescribeRegionsRequest();
    auto future = client->callAsync<DescribeRegionsOutcome>(&OSSAsyncClient::describeRegionsAsync, request);
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());

    auto& error = outcome.getError();
    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("DescribeRegions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ(24, error.getRequestId().size());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
