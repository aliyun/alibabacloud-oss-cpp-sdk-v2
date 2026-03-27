
#pragma once
#include "alibabacloud/oss2/Types.h"

namespace alibabacloud {
namespace oss2 {
void InitLogInner();
void DeinitLogInner();

LogLevel GetLogLevelInner();
LogCallback GetLogCallbackInner();
void SetLogLevelInner(LogLevel level);
void SetLogCallbackInner(LogCallback callback);

void FormattedLog(LogLevel logLevel, const char* tag, const char* formatStr, ...);

#ifdef DISABLE_OSS_LOGGING

#define OSS_LOG(level, tag, ...)

#else

#define OSS_LOG(level, tag, ...)                                                                            \
    {                                                                                                       \
        if (alibabacloud::oss2::GetLogCallbackInner() && alibabacloud::oss2::GetLogLevelInner() >= level) { \
            FormattedLog(level, tag, __VA_ARGS__);                                                          \
        }                                                                                                   \
    }

#endif // DISABLE_OSS_LOGGING
} // namespace oss2
} // namespace alibabacloud
