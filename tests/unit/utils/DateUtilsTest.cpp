#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(DateUtilsTest, ToGmtTimeTest) {
    std::time_t t = 0;
    std::string timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Thu, 01 Jan 1970 00:00:00 GMT");

    t = 1520411719;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Wed, 07 Mar 2018 08:35:19 GMT");

    t = 1554703347;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 06:02:27 GMT");

    t = 1554739347;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 16:02:27 GMT");
}

TEST(DateUtilsTest, ToGmtTimeWithSetlocaleTest) {
    auto oldLoc = std::cout.getloc();
    std::locale::global(std::locale(""));

    std::time_t t = 0;
    std::string timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Thu, 01 Jan 1970 00:00:00 GMT");

    t = 1520411719;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Wed, 07 Mar 2018 08:35:19 GMT");

    t = 1554703347;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 06:02:27 GMT");

    t = 1554739347;
    timeStr = ToGmtTime(t);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 16:02:27 GMT");

    std::locale::global(oldLoc);
}

TEST(DateUtilsTest, ToUtcTimeTest) {
    std::time_t t = 0;
    std::string timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");
}

TEST(DateUtilsTest, ToUtcTimeWithSetlocaleTest) {
    auto oldLoc = std::cout.getloc();
    std::locale::global(std::locale(""));

    std::time_t t = 0;
    std::string timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

    t = 1520433319;
    timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T14:35:19.000Z");

    std::locale::global(oldLoc);
}

TEST(DateUtilsTest, UtcToUnixTimeTest) {
    std::string date = "1970-01-01T00:00:00.000Z";
    std::time_t t = UtcToUnixTime(date);
    EXPECT_EQ(t, 0);

    date = "2018-03-07T08:35:19.123Z";
    t = UtcToUnixTime(date);
    EXPECT_EQ(t, 1520411719);

    // invalid case
    date = "2018-03-07T08:35:19Z";
    t = UtcToUnixTime(date);
    EXPECT_EQ(t, -1);

    date = "2018-03-07T08:35:19.abcZ";
    t = UtcToUnixTime(date);
    EXPECT_EQ(t, -1);

    date = "18-03-07T08:35:19.000Z";
    t = UtcToUnixTime(date);
    EXPECT_EQ(t, -1);

    date = "";
    t = UtcToUnixTime(date);
    EXPECT_EQ(t, -1);
}


TEST(DateUtilsTest, FormatUnixTimeTest) {
    // GMT
    std::string gmt_foramt = "%a, %d %b %Y %H:%M:%S GMT";
    std::time_t t = 0;
    std::string timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Thu, 01 Jan 1970 00:00:00 GMT");

    t = 1520411719;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Wed, 07 Mar 2018 08:35:19 GMT");

    t = 1554703347;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 06:02:27 GMT");

    t = 1554739347;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 16:02:27 GMT");

    auto oldLoc = std::cout.getloc();
    std::locale::global(std::locale(""));

    t = 0;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Thu, 01 Jan 1970 00:00:00 GMT");

    t = 1520411719;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Wed, 07 Mar 2018 08:35:19 GMT");

    t = 1554703347;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 06:02:27 GMT");

    t = 1554739347;
    timeStr = FormatUnixTime(t, gmt_foramt);
    EXPECT_STREQ(timeStr.c_str(), "Mon, 08 Apr 2019 16:02:27 GMT");

    std::locale::global(oldLoc);

    // UTC
    std::string utc_foramt = "%Y-%m-%dT%H:%M:%S.000Z";
    t = 0;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

    oldLoc = std::cout.getloc();
    std::locale::global(std::locale(""));

    t = 0;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

    t = 1520433319;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T14:35:19.000Z");

    std::locale::global(oldLoc);


    // V4 TIME FORMAT
}

TEST(DateUtilsTest, ToUnixTimeTest) {
    std::string utc_foramt = "%Y-%m-%dT%H:%M:%S";
    std::string date = "1970-01-01T00:00:00.000Z";
    std::time_t t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, 0);

    date = "2018-03-07T08:35:19.123Z";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, 1520411719);

    date = "2018-03-07T08:35:19Z";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, 1520411719);

    date = "2018-03-07T08:35:19.abcZ";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, 1520411719);

    date = "18-03-07T08:35:19.000Z";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, -1);

    // invalid case
    date = "ab-03-07T08:35:19.000Z";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, -1);

    date = "";
    t = ToUnixTime(date, utc_foramt);
    EXPECT_EQ(t, -1);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud