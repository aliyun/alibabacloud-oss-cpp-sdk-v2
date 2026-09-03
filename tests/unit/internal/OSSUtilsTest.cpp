#include <gtest/gtest.h>

#include "src/internal/OSSUtils.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

TEST(OSSUtilsTest, IsValidIpTest) {
    EXPECT_EQ(isValidIp("192.168.1.1"), true);
    EXPECT_EQ(isValidIp("www.test-inc.com"), false);
    EXPECT_EQ(isValidIp("WWW.test-inc_CN.com"), false);
}


TEST(OSSUtilsTest, IsValidMethod) {
    std::vector<std::string> metholds = {"PUT", "GET", "POST", "HEAD", "DELETE", "OPTIONS"};

    for (const auto& item : metholds) {
        EXPECT_EQ(true, isValidMethod(item));
    }

    EXPECT_EQ(false, isValidMethod(""));
    EXPECT_EQ(false, isValidMethod("123"));
}


TEST(OSSUtilsTest, IsValidAccountId) {
    EXPECT_EQ(true, isValidAccountId("0"));
    EXPECT_EQ(true, isValidAccountId("1234567890"));
    EXPECT_EQ(false, isValidAccountId(""));
    EXPECT_EQ(false, isValidAccountId("123abc"));
    EXPECT_EQ(false, isValidAccountId("12 34"));
    EXPECT_EQ(false, isValidAccountId("-123"));
}


TEST(OSSUtilsTest, BuildHostPathAddressStyles) {
    OperationInput input;
    input.opName = "PutObject";
    input.method = "PUT";
    input.bucket = "bucket";
    input.key = "key";

    const std::string baseUrl = "oss-cn-hangzhou.aliyuncs.com";

    EXPECT_EQ("bucket.oss-cn-hangzhou.aliyuncs.com/key",
              buildHostPath(input, baseUrl, AddressStyleType::VirtualHosted));
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com/bucket/key", buildHostPath(input, baseUrl, AddressStyleType::Path));
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com/key", buildHostPath(input, baseUrl, AddressStyleType::CName));

    // VirtualHostedAlias is agentic-only, the plain client falls back to virtual-hosted.
    EXPECT_EQ("bucket.oss-cn-hangzhou.aliyuncs.com/key",
              buildHostPath(input, baseUrl, AddressStyleType::VirtualHostedAlias));
}


} // namespace internal
} // namespace oss2
} // namespace alibabacloud