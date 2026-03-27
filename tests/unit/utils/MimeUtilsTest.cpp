#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(MimeUtilsTest, LookupMimeTypeTest) {
    EXPECT_STREQ(LookupMimeType("name.html").c_str(), "text/html");
    EXPECT_STREQ(LookupMimeType("test.mp3").c_str(), "audio/mpeg");
    EXPECT_STREQ(LookupMimeType("test.mp3.unkonw").c_str(), "audio/mpeg");
    EXPECT_STREQ(LookupMimeType("test.mp3.unkonw.unkonw").c_str(), "application/octet-stream");
    EXPECT_STREQ(LookupMimeType("unkonw").c_str(), "application/octet-stream");
    EXPECT_STREQ(LookupMimeType("name.Html").c_str(), "text/html");
    EXPECT_STREQ(LookupMimeType("test.Mp3.unkonw").c_str(), "audio/mpeg");
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud