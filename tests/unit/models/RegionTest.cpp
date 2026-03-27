#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/Region.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(RegionTest, DescribeRegionsRequest_ConstructorDefault) {
    // Default
    auto request = DescribeRegionsRequest();
    EXPECT_EQ("", request.getRegions());
}

TEST(RegionTest, DescribeRegionsRequest_Setter) {
    auto request = DescribeRegionsRequest();
    EXPECT_EQ("", request.getRegions());

    // Setter
    request.setRegions("oss-cn-hangzhou");

    EXPECT_EQ("oss-cn-hangzhou", request.getRegions());
}

TEST(RegionTest, DescribeRegionsResult_ConstructorDefault) {
    auto result = DescribeRegionsResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasRegionInfoList());
    EXPECT_EQ(0, result.getRegionInfoList().regionInfos.size());
}

TEST(RegionTest, ListBucketsResult_ConstructorAll) {
    auto result = DescribeRegionsResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasRegionInfoList());
    EXPECT_EQ(0, result.getRegionInfoList().regionInfos.size());
}

TEST(RegionTest, ListBucketsResult_SetBody) {
    auto result = DescribeRegionsResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    result.setRegionInfoList(
            // RegionInfoList
            {{
                    // RegionInfo
                    {"oss-ap-northeast-1.aliyuncs.com", "oss-ap-northeast-1-internal.aliyuncs.com",
                     "oss-accelerate.aliyuncs.com", "oss-ap-northeast-1"},
                    // RegionInfo
                    {"oss-ap-northeast-2.aliyuncs.com", "oss-ap-northeast-2-internal.aliyuncs.com",
                     "oss-accelerate.aliyuncs.com", "oss-ap-northeast-2"},
            }});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasRegionInfoList());
    EXPECT_EQ(2, result.getRegionInfoList().regionInfos.size());
    EXPECT_EQ("oss-ap-northeast-1", result.getRegionInfoList().regionInfos.at(0).region);
    EXPECT_EQ("oss-ap-northeast-1.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internetEndpoint);
    EXPECT_EQ("oss-ap-northeast-1-internal.aliyuncs.com",
              result.getRegionInfoList().regionInfos.at(0).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).accelerateEndpoint);
    EXPECT_EQ("oss-ap-northeast-2", result.getRegionInfoList().regionInfos.at(1).region);
}

TEST(RegionTest, RegionInfoList_ConstructorAll) {
    auto value = RegionInfoList();
    EXPECT_EQ(0, value.regionInfos.size());

    auto info = RegionInfo{};
    info.setRegion("oss-ap-northeast-1");
    info.setInternetEndpoint("oss-ap-northeast-1.aliyuncs.com");
    info.setInternalEndpoint("oss-ap-northeast-1-internal.aliyuncs.com");
    info.setAccelerateEndpoint("oss-accelerate.aliyuncs.com");

    value.setRegionInfos({info});
    EXPECT_EQ(1, value.regionInfos.size());
    EXPECT_EQ("oss-ap-northeast-1", value.regionInfos.at(0).region);
    EXPECT_EQ("oss-ap-northeast-1.aliyuncs.com", value.regionInfos.at(0).internetEndpoint);
    EXPECT_EQ("oss-ap-northeast-1-internal.aliyuncs.com", value.regionInfos.at(0).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", value.regionInfos.at(0).accelerateEndpoint);
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud