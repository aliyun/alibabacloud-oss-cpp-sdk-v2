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

TEST(MimeUtilsTest, AddMimeType_NewExtension) {
    addMimeType({{"parquet", "application/x-parquet"}});
    EXPECT_EQ(LookupMimeType("data.parquet"), "application/x-parquet");
    clearMimeType();
}

TEST(MimeUtilsTest, AddMimeType_OverrideBuiltin) {
    addMimeType({{"html", "text/html; charset=utf-8"}});
    EXPECT_EQ(LookupMimeType("page.html"), "text/html; charset=utf-8");
    clearMimeType();
    EXPECT_EQ(LookupMimeType("page.html"), "text/html");
}

TEST(MimeUtilsTest, AddMimeType_MultipleExtensions) {
    addMimeType({{"avro", "application/avro"}, {"orc", "application/x-orc"}});
    EXPECT_EQ(LookupMimeType("file.avro"), "application/avro");
    EXPECT_EQ(LookupMimeType("file.orc"), "application/x-orc");
    clearMimeType();
}

TEST(MimeUtilsTest, AddMimeType_SecondaryExtensionFallback) {
    addMimeType({{"custom", "application/x-custom"}});
    EXPECT_EQ(LookupMimeType("archive.custom.unknown"), "application/x-custom");
    clearMimeType();
}

TEST(MimeUtilsTest, AddMimeType_OverwriteExistingUserMapping) {
    addMimeType({{"myext", "type/first"}});
    EXPECT_EQ(LookupMimeType("file.myext"), "type/first");
    addMimeType({{"myext", "type/second"}});
    EXPECT_EQ(LookupMimeType("file.myext"), "type/second");
    clearMimeType();
}

TEST(MimeUtilsTest, ClearMimeType_RestoresDefaults) {
    addMimeType({{"html", "custom/html"}, {"newext", "custom/new"}});
    clearMimeType();
    EXPECT_EQ(LookupMimeType("page.html"), "text/html");
    EXPECT_EQ(LookupMimeType("file.newext"), "application/octet-stream");
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud