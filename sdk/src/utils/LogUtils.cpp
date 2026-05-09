#include "LogUtils.h"
#include "Utils.h"
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>


namespace alibabacloud {
namespace oss2 {

static LogLevel gOssLogLevel = LogLevel::LogOff;
static LogCallback gLogCallback = nullptr;
const static char* EnvLogLevels[] = {"off", "fatal", "error", "warn", "info", "debug", "trace", "all"};

static std::string LogPrefix(LogLevel logLevel, const char* tag) {
    static const char* LogStr[] = {"[OFF]", "[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]", "[ALL]"};
    int index = (int) logLevel - (int) LogLevel::LogOff;
    std::stringstream ss;
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto ms = tp.time_since_epoch().count() % 1000;
    auto t = std::chrono::system_clock::to_time_t(tp);
    struct tm tm{};
#ifdef _WIN32
    ::localtime_s(&tm, &t);
#else
    ::localtime_r(&t, &tm);
#endif

#if defined(__GNUG__) && __GNUC__ < 5
    char tmbuff[64];
    strftime(tmbuff, 64, "%Y-%m-%d %H:%M:%S.", &tm);
    ss << "[" << tmbuff << std::setw(3) << std::setfill('0') << ms << "]";
#else
    ss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.") << std::setw(3) << std::setfill('0') << ms << "]";
#endif
    ss << LogStr[index];
    ss << "[" << tag << "]";
    ss << "[" << std::this_thread::get_id() << "]";
    return ss.str();
}

void FormattedLog(LogLevel logLevel, const char* tag, const char* fmt, ...) {
    std::stringstream ss;
    ss << LogPrefix(logLevel, tag);
    char buffer[2050];
    int i = 0;
    va_list args;
    va_start(args, fmt);
#ifdef WIN32
    i = vsnprintf_s(buffer, sizeof(buffer) - 1, _TRUNCATE, fmt, args);
#else
    i = vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
#endif
    va_end(args);

    while (i > 0 && buffer[i - 1] == '\n') {
        i--;
        buffer[i] = '\0';
    }

    ss << buffer << std::endl;
    if (gLogCallback) {
        gLogCallback(logLevel, ss.str());
    }
}

static void DefaultLogCallbackFunc(LogLevel level, const std::string& stream) {
    ((void) (level));
    std::cerr << stream;
}

LogLevel GetLogLevelInner() {
    return gOssLogLevel;
}

LogCallback GetLogCallbackInner() {
    return gLogCallback;
}

void SetLogLevelInner(LogLevel level) {
    gOssLogLevel = level;
}

void SetLogCallbackInner(LogCallback callback) {
    gLogCallback = callback;
}

void InitLogInner() {
    gOssLogLevel = LogLevel::LogOff;
    gLogCallback = nullptr;
    auto value = std::getenv("OSS_SDK_LOG_LEVEL");
    if (value) {
        auto level = utils::ToLower(utils::Trim(value).c_str());
        const auto size = sizeof(EnvLogLevels) / sizeof(EnvLogLevels[0]);
        for (auto i = 0U; i < size; i++) {
            if (level.compare(EnvLogLevels[i]) == 0) {
                gOssLogLevel = static_cast<decltype(LogLevel::LogOff)>(static_cast<decltype(i)>(LogLevel::LogOff) + i);
                gLogCallback = DefaultLogCallbackFunc;
                break;
            }
        }
    }
}

void DeinitLogInner() {
    gOssLogLevel = LogLevel::LogOff;
    gLogCallback = nullptr;
}
} // namespace oss2
} // namespace alibabacloud