#include <gtest/gtest.h>

#include "src/utils/Utils.h"

// macOS/MinGW + GCC(libstdc++): std::locale("") crashes because libstdc++
// doesn't recognise the system locale names, and -fno-exceptions turns the
// throw into std::terminate(). Fall back to the C API setlocale().
#if defined(__GNUC__) && !defined(__clang__) && (defined(__APPLE__) || defined(__MINGW32__))
#include <clocale>
#define SET_LOCALE_ENV()                                        \
    const char* _oldLoc = setlocale(LC_ALL, nullptr);           \
    std::string _savedLoc = _oldLoc ? _oldLoc : "C";           \
    setlocale(LC_ALL, "")
#define RESTORE_LOCALE()                                        \
    setlocale(LC_ALL, _savedLoc.c_str())
#else
#define SET_LOCALE_ENV()                                        \
    auto _savedLoc = std::cout.getloc();                        \
    std::locale::global(std::locale(""))
#define RESTORE_LOCALE()                                        \
    std::locale::global(_savedLoc)
#endif

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
    SET_LOCALE_ENV();

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

    RESTORE_LOCALE();
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
    SET_LOCALE_ENV();

    std::time_t t = 0;
    std::string timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

    t = 1520433319;
    timeStr = ToUtcTime(t);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T14:35:19.000Z");

    RESTORE_LOCALE();
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

    {
        SET_LOCALE_ENV();

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

        RESTORE_LOCALE();
    }

    // UTC
    std::string utc_foramt = "%Y-%m-%dT%H:%M:%S.000Z";
    t = 0;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

    t = 1520411719;
    timeStr = FormatUnixTime(t, utc_foramt);
    EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

    {
        SET_LOCALE_ENV();

        t = 0;
        timeStr = FormatUnixTime(t, utc_foramt);
        EXPECT_STREQ(timeStr.c_str(), "1970-01-01T00:00:00.000Z");

        t = 1520411719;
        timeStr = FormatUnixTime(t, utc_foramt);
        EXPECT_STREQ(timeStr.c_str(), "2018-03-07T08:35:19.000Z");

        t = 1520433319;
        timeStr = FormatUnixTime(t, utc_foramt);
        EXPECT_STREQ(timeStr.c_str(), "2018-03-07T14:35:19.000Z");

        RESTORE_LOCALE();
    }


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

TEST(DateUtilsTest, GmtToUnixTimeTest) {
    EXPECT_EQ(GmtToUnixTime("Thu, 01 Jan 1970 00:00:00 GMT"), 0);
    EXPECT_EQ(GmtToUnixTime("Wed, 07 Mar 2018 08:35:19 GMT"), 1520411719);
    EXPECT_EQ(GmtToUnixTime("Mon, 08 Apr 2019 06:02:27 GMT"), 1554703347);
    EXPECT_EQ(GmtToUnixTime("Mon, 08 Apr 2019 16:02:27 GMT"), 1554739347);

    EXPECT_EQ(GmtToUnixTime(""), -1);
    EXPECT_EQ(GmtToUnixTime("garbage"), -1);
    EXPECT_EQ(GmtToUnixTime("2018-03-07T08:35:19.000Z"), -1);
}

TEST(DateUtilsTest, GmtToUnixTimeWithSetlocaleTest) {
    SET_LOCALE_ENV();

    EXPECT_EQ(GmtToUnixTime("Thu, 01 Jan 1970 00:00:00 GMT"), 0);
    EXPECT_EQ(GmtToUnixTime("Wed, 07 Mar 2018 08:35:19 GMT"), 1520411719);
    EXPECT_EQ(GmtToUnixTime("Mon, 08 Apr 2019 06:02:27 GMT"), 1554703347);

    RESTORE_LOCALE();
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud