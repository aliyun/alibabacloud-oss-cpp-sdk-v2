#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(Base64UtilsTest, Encode) {
    std::vector<std::string> ori = {"abc", "abcd", "abcde", ""};
    std::vector<std::string> pat = {"YWJj", "YWJjZA==", "YWJjZGU=", ""};

    auto i = ori.size();
    for (i = 0; i < ori.size(); i++) {
        auto result = Base64Encode(ori[i]);
        EXPECT_STREQ(result.c_str(), pat[i].c_str());
    }
    EXPECT_TRUE((i == ori.size()));
}

TEST(Base64UtilsTest, Decode) {
    std::vector<std::string> ori = {"YWJj", "YWJjZA==", "YWJjZGU=", "", "YWJjZA", "YWJjZGU"};
    std::vector<std::string> pat = {"abc", "abcd", "abcde", "", "abcd", "abcde"};
    auto i = ori.size();
    for (i = 0; i < ori.size(); i++) {
        auto result = Base64Decode(ori[i]);
        EXPECT_EQ(pat[i], result);
    }
}


TEST(Base64UtilsTest, Base64EncodeTest) {
    std::vector<std::string> ori = {"abc", "abcd", "abcde", ""};
    std::vector<std::string> pat = {"YWJj", "YWJjZA==", "YWJjZGU=", ""};

    auto i = ori.size();
    for (i = 0; i < ori.size(); i++) {
        auto result = Base64Encode(ori[i]);
        EXPECT_STREQ(result.c_str(), pat[i].c_str());
    }
    EXPECT_TRUE((i == ori.size()));
}

TEST(Base64UtilsTest, Base64DecodeTest) {
    std::vector<std::string> ori = {"YWJj", "YWJjZA==", "YWJjZGU=", "", "YWJjZA", "YWJjZGU"};
    std::vector<std::string> pat = {"abc", "abcd", "abcde", "", "abcd", "abcde"};

    auto i = ori.size();
    for (i = 0; i < ori.size(); i++) {
        auto result = Base64Decode(ori[i]);
        EXPECT_EQ(result.size(), pat[i].size());
    }
    EXPECT_TRUE((i == ori.size()));
}

TEST(Base64UtilsTest, Base64EncodeUrlSafeTest) {
    const unsigned char buff[] = {0x14, 0xFB, 0x9C, 0x03, 0xD9, 0x7E};
    size_t len = sizeof(buff) / sizeof(buff[0]);
    auto value = Base64EncodeUrlSafe((std::byte*) buff, static_cast<int>(len));
    EXPECT_EQ(value, "FPucA9l-");

    std::vector<std::string> ori = {"abc", "abcd", "abcde"};
    std::vector<std::string> pat = {"YWJj", "YWJjZA", "YWJjZGU"};

    auto i = ori.size();
    for (i = 0; i < ori.size(); i++) {
        auto result = Base64EncodeUrlSafe(ori[i]);
        EXPECT_EQ(result, pat[i]);
    }
    EXPECT_TRUE((i == ori.size()));
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud