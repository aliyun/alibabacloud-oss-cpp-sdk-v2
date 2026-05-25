#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

static std::vector<std::string> urlOri = 
{
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~",
    "`!@#$%^&*()+={}[]:;'\\|<>,?/ \"",
    "hello world!"
};

static std::vector<std::string> urlPat =
{
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~",
    "%60%21%40%23%24%25%5E%26%2A%28%29%2B%3D%7B%7D%5B%5D%3A%3B%27%5C%7C%3C%3E%2C%3F%2F%20%22",
    "hello%20world%21"
};

static std::vector<std::string> urlPatIgnoreSlash =
{
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~",
    "%60%21%40%23%24%25%5E%26%2A%28%29%2B%3D%7B%7D%5B%5D%3A%3B%27%5C%7C%3C%3E%2C%3F/%20%22",
    "hello%20world%21"
};

TEST(HttpUtilsTest, UrlEncodeTest)
{
    auto i = urlOri.size();
    for (i = 0; i < urlOri.size(); i++) {
        auto result = UrlEncode(urlOri[i]);
        EXPECT_STREQ(result.c_str(), urlPat[i].c_str());
    }
    EXPECT_TRUE((i == urlOri.size()));
}

TEST(HttpUtilsTest, UrlEncodePathTest)
{
    auto i = urlOri.size();
    for (i = 0; i < urlOri.size(); i++) {
        auto result = UrlEncodePath(urlOri[i]);
        EXPECT_STREQ(result.c_str(), urlPatIgnoreSlash[i].c_str());
    }
    EXPECT_TRUE((i == urlOri.size()));

    for (i = 0; i < urlOri.size(); i++) {
        auto result = UrlEncode(urlOri[i]);
        EXPECT_STREQ(result.c_str(), urlPat[i].c_str());
    }
    EXPECT_TRUE((i == urlOri.size()));
}


TEST(HttpUtilsTest, UrlDecodeTest)
{
    auto i = urlOri.size();
    for (i = 0; i < urlOri.size(); i++) {
        auto result = UrlDecode(urlPat[i]);
        EXPECT_STREQ(result.c_str(), urlOri[i].c_str());
    }
    EXPECT_TRUE((i == urlPat.size()));
}

TEST(HttpUtilsTest, UrlDecodePercentAtEnd) {
    std::string input = "hello%";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "hello%");
}

TEST(HttpUtilsTest, UrlDecodePercentWithOneChar) {
    std::string input = "hello%A";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "hello%");
}

TEST(HttpUtilsTest, UrlDecodeSinglePercent) {
    std::string input = "%";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "%");
}

TEST(HttpUtilsTest, UrlDecodeTwoCharsEndingWithPercent) {
    std::string input = "x%";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "x%");
}

TEST(HttpUtilsTest, UrlDecodeConsecutivePercents) {
    std::string input = "%%";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "%");
}

TEST(HttpUtilsTest, UrlDecodePercentWithNonHex) {
    std::string input = "a%b";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "a%");
}

TEST(HttpUtilsTest, UrlDecodePercentThenNonHex) {
    std::string input = "%G";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "%");
}

TEST(HttpUtilsTest, UrlDecodePercentThenTwoNonHex) {
    std::string input = "%GG";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "%GG");
}

TEST(HttpUtilsTest, UrlDecodeValidThenInvalid) {
    std::string input = "a%20b%c";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "a b%");
}

TEST(HttpUtilsTest, UrlDecodePercentThenEncodedSpace) {
    std::string input = "%%20";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "% ");
}

TEST(HttpUtilsTest, UrlDecodeValidEncodedChars) {
    std::string input = "%20";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), " ");
}

TEST(HttpUtilsTest, UrlDecodeValidString) {
    std::string input = "hello%20world";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "hello world");
}

TEST(HttpUtilsTest, UrlDecodeMultipleEncodedChars) {
    std::string input = "%48%65%6C%6C%6F";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "Hello");
}

TEST(HttpUtilsTest, UrlDecodeLowercaseHex) {
    std::string input = "%2f";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "/");
}

TEST(HttpUtilsTest, UrlDecodeMixedCaseHex) {
    std::string input = "%2F%3a%3A";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "/::");
}

TEST(HttpUtilsTest, UrlDecodeSpecialChars) {
    std::string input = "%21%40%23%24%25";
    std::string result = UrlDecode(input);
    EXPECT_STREQ(result.c_str(), "!@#$%");
}

// --- ParseRangeHeader ---

TEST(HttpUtilsTest, ParseRangeHeader_SimpleRange) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_TRUE(ParseRangeHeader("bytes=0-499", ranges));
    ASSERT_EQ(1u, ranges.size());
    EXPECT_EQ(0, ranges[0].first);
    EXPECT_EQ(499, ranges[0].second);
}

TEST(HttpUtilsTest, ParseRangeHeader_OpenEnd) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_TRUE(ParseRangeHeader("bytes=500-", ranges));
    ASSERT_EQ(1u, ranges.size());
    EXPECT_EQ(500, ranges[0].first);
    EXPECT_EQ(-1, ranges[0].second);
}

TEST(HttpUtilsTest, ParseRangeHeader_SuffixRange) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_TRUE(ParseRangeHeader("bytes=-500", ranges));
    ASSERT_EQ(1u, ranges.size());
    EXPECT_EQ(-1, ranges[0].first);
    EXPECT_EQ(500, ranges[0].second);
}

TEST(HttpUtilsTest, ParseRangeHeader_MultipleRanges) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_TRUE(ParseRangeHeader("bytes=0-99,200-299,400-", ranges));
    ASSERT_EQ(3u, ranges.size());
    EXPECT_EQ(0, ranges[0].first);
    EXPECT_EQ(99, ranges[0].second);
    EXPECT_EQ(200, ranges[1].first);
    EXPECT_EQ(299, ranges[1].second);
    EXPECT_EQ(400, ranges[2].first);
    EXPECT_EQ(-1, ranges[2].second);
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidNoPrefix) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("0-499", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidEmpty) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidBothEmpty) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=-", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidStartGreaterThanEnd) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=500-100", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidNonDigit) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=abc-def", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_InvalidNoDash) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=100", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_TooShort) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_Overflow) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=99999999999999999999-", ranges));
    EXPECT_TRUE(ranges.empty());
}

TEST(HttpUtilsTest, ParseRangeHeader_WhitespaceAroundRange) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_TRUE(ParseRangeHeader("bytes= 0-99 ", ranges));
    ASSERT_EQ(1u, ranges.size());
    EXPECT_EQ(0, ranges[0].first);
    EXPECT_EQ(99, ranges[0].second);
}

TEST(HttpUtilsTest, ParseRangeHeader_NegativeNumber) {
    std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
    EXPECT_FALSE(ParseRangeHeader("bytes=-1-2", ranges));
    EXPECT_TRUE(ranges.empty());
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud