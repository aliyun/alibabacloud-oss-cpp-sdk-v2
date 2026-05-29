
#include "Utils.h"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace utils {

std::string ToGmtTime(std::time_t& t) {
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif

#if defined(__GNUG__) && __GNUC__ < 5
    static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char tmbuff[26];
    snprintf(tmbuff, sizeof(tmbuff), "%.3s, %.2d %.3s %d %.2d:%.2d:%.2d", wday_name[tm.tm_wday], tm.tm_mday,
             mon_name[tm.tm_mon], 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
    date << tmbuff << " GMT";
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
#endif
    return date.str();
}

std::string ToUtcTime(std::time_t& t) {
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif
#if defined(__GNUG__) && __GNUC__ < 5
    char tmbuff[26];
    strftime(tmbuff, 26, "%Y-%m-%dT%H:%M:%S.000Z", &tm);
    date << tmbuff;
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%Y-%m-%dT%X.000Z");
#endif
    return date.str();
}

std::time_t UtcToUnixTime(const std::string& t) {
    const char* date = t.c_str();
    std::tm tm;
    std::time_t tt = -1;
    int ms;
    auto result = sscanf(date, "%4d-%2d-%2dT%2d:%2d:%2d.%dZ", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour,
                         &tm.tm_min, &tm.tm_sec, &ms);

    if (result == 7) {
        tm.tm_year = tm.tm_year - 1900;
        tm.tm_mon = tm.tm_mon - 1;
#ifdef _WIN32
        tt = _mkgmtime64(&tm);
#else
        tt = timegm(&tm);
#endif // _WIN32
    }
    return tt < 0 ? -1 : tt;
}

std::time_t GmtToUnixTime(const std::string& s) {
    if (s.size() < 4 || s.compare(s.size() - 4, 4, " GMT") != 0) {
        return -1;
    }
    return ToUnixTime(s, "%a, %d %b %Y %H:%M:%S");
}

std::time_t ToUnixTime(const std::string& str, const std::string& fmt) {
    std::tm tm;
    std::time_t tt = -1;
    memset(&tm, 0, sizeof(tm));
#if defined(__GNUG__) && __GNUC__ < 5
    strptime(str.c_str(), fmt.c_str(), &tm);
#else
    std::istringstream input(str);
    input.imbue(std::locale::classic());
    input >> std::get_time(&tm, fmt.c_str());
    if (input.fail()) {
        return -1;
    }
#endif

#ifdef _WIN32
    tt = _mkgmtime64(&tm);
#else
    tt = timegm(&tm);
#endif // _WIN32
    return tt < 0 ? -1 : tt;
}

std::string FormatUnixTime(const std::time_t& t, const std::string& fmt) {
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif
#if defined(__GNUG__) && __GNUC__ < 5
    char tmbuff[64];
    strftime(tmbuff, 64, fmt.c_str(), &tm);
    date << tmbuff;
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, fmt.c_str());
#endif
    return date.str();
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud