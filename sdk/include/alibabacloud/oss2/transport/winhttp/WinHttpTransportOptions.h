#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {

struct ALIBABACLOUD_OSS_API WinHttpTransportOptions : HttpTransportOptions {
    // Connection pool size per host, default 16
    std::optional<unsigned int> poolSize;
    // Proxy server port
    std::optional<unsigned int> proxyPort;
    // Proxy authentication username
    std::optional<std::string> proxyUserName;
    // Proxy authentication password
    std::optional<std::string> proxyPassword;
};

} // namespace oss2
} // namespace alibabacloud
