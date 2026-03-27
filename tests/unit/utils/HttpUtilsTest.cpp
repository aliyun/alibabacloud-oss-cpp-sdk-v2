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

} // namespace utils
} // namespace oss2
} // namespace alibabacloud