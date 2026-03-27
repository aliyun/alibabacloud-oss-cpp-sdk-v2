#include <gtest/gtest.h>

#include "Config.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {

TEST(RegionTest, DescribeRegions_normal) {
    auto client = Config::GetDefaultClient();

    // all
    auto request = models::DescribeRegionsRequest();
    auto outcome = client->describeRegions(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(24, result.getRequestId().size());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(true, result.hasRegionInfoList());
    EXPECT_TRUE(result.getRegionInfoList().regionInfos.size() > 0);
    EXPECT_EQ("oss-ap-northeast-1", result.getRegionInfoList().regionInfos.at(0).region);
    EXPECT_EQ("oss-ap-northeast-1.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internetEndpoint);
    EXPECT_EQ("oss-ap-northeast-1-internal.aliyuncs.com",
              result.getRegionInfoList().regionInfos.at(0).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).accelerateEndpoint);
    EXPECT_EQ("oss-ap-northeast-2", result.getRegionInfoList().regionInfos.at(1).region);

    // oss-cn-hangzhou
    request.setRegions("oss-cn-hangzhou");
    outcome = client->describeRegions(request);
    result = outcome.getResult();
    EXPECT_EQ(1, result.getRegionInfoList().regionInfos.size());
    EXPECT_EQ("oss-cn-hangzhou", result.getRegionInfoList().regionInfos.at(0).region);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou-internal.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).accelerateEndpoint);
}

TEST(RegionTest, DescribeRegions_fail) {
    auto client = Config::GetInvalidClient();

    auto request = models::DescribeRegionsRequest();
    auto outcome = client->describeRegions(request);
    EXPECT_FALSE(outcome.isSuccess());

    auto& error = outcome.getError();
    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("DescribeRegions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ(24, error.getRequestId().size());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
}

} // namespace oss2
} // namespace alibabacloud
