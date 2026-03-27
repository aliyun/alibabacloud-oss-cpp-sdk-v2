#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(StringUtilsTest, TrimSpaceChTest) {
    std::string str = " 11  ";
    EXPECT_STREQ(Trim(str.c_str()).c_str(), "11");

    str = " 11  22  ";
    EXPECT_STREQ(Trim(str.c_str()).c_str(), "11  22");
}

TEST(StringUtilsTest, TrimSpaceTest) {
    std::vector<std::string> testString = {" abc ", "   abc   ",    "  abc",
                                           "abc  ", "    abc     ", "\r    \nabc     \n\r"};
    for (auto const& str : testString) {
        EXPECT_STREQ(Trim(str.c_str()).c_str(), "abc");
    }
}

TEST(StringUtilsTest, LeftTrimSpaceTest) {
    std::vector<std::string> testString = {" abc ", "   abc   ", "  abc    ", "abc  "};
    for (auto const& str : testString) {
        auto result = LeftTrim(str.c_str());
        EXPECT_EQ(result.compare(0, 3, "abc", 3), 0);
        EXPECT_NE(result.compare("abc"), 0);
    }
}

TEST(StringUtilsTest, RightTrimSpaceTest) {
    std::vector<std::string> testString = {" abc ", "   abc   ", "  abc    ", "abc  "};
    for (auto const& str : testString) {
        auto result = RightTrim(str.c_str());
        auto pos = result.find('a');
        EXPECT_EQ(strcmp(result.c_str() + pos, "abc"), 0);
    }
}

TEST(StringUtilsTest, TrimQuotesTest) {
    std::vector<std::string> testString = {R"("abc")", R"(""abc"")", R"(""abc)", R"(abc"""")"};
    for (auto const& str : testString) {
        EXPECT_STREQ(TrimQuotes(str.c_str()).c_str(), "abc");
    }
}

TEST(StringUtilsTest, LeftTrimQuotesTest) {
    std::vector<std::string> testString = {R"(""abc"")", R"("""abc "")", R"("""abc  "")", R"(abc  "")"};
    for (auto const& str : testString) {
        auto result = LeftTrimQuotes(str.c_str());
        EXPECT_EQ(result.compare(0, 3, "abc", 3), 0);
        EXPECT_NE(result.compare("abc"), 0);
    }
}

TEST(StringUtilsTest, RightTrimQuotesTest) {
    std::vector<std::string> testString = {R"(""abc"")", R"(""" abc"")", R"("""abc")", R"(abc""""")"};
    for (auto const& str : testString) {
        auto result = RightTrimQuotes(str.c_str());
        auto pos = result.find('a');
        EXPECT_EQ(strcmp(result.c_str() + pos, "abc"), 0);
    }
}

TEST(StringUtilsTest, ToLowerTest) {
    std::vector<std::string> testString = {"ABC", "Abc", "AbC", "abc"};
    for (auto const& str : testString) {
        auto result = ToLower(str.c_str());
        EXPECT_STREQ(result.c_str(), "abc");
    }
}

TEST(StringUtilsTest, ToUpperTest) {
    std::vector<std::string> testString = {"ABC", "Abc", "AbC", "abc"};
    for (auto const& str : testString) {
        auto result = ToUpper(str.c_str());
        EXPECT_STREQ(result.c_str(), "ABC");
    }
}


TEST(StringUtilsTest, StringReplaceTest)
{
    std::string test = "1234abcdABCD1234";

    StringReplace(test, "abcd", "A");
    EXPECT_EQ(test, "1234AABCD1234");

    test = "12212";
    StringReplace(test, "12", "21");
    EXPECT_EQ(test, "21221");
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud