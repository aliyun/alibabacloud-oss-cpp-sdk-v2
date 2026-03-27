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


} // namespace internal
} // namespace oss2
} // namespace alibabacloud