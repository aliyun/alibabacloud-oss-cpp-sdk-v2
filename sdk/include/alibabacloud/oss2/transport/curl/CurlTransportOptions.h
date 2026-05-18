#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <functional>
#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {

struct ALIBABACLOUD_OSS_API CurlTransportOptions : HttpTransportOptions {
    // Max concurrent connections per host, default: sync 16, async 100
    std::optional<unsigned int> maxConnections;
    // CA certificate directory path (CURLOPT_CAPATH)
    std::optional<std::string> caPath;
    // CA certificate bundle file path (CURLOPT_CAINFO)
    std::optional<std::string> caFile;
    // Network interface to bind, e.g. "eth0" (CURLOPT_INTERFACE)
    std::optional<std::string> networkInterface;
    // Proxy server port (CURLOPT_PROXYPORT)
    std::optional<unsigned int> proxyPort;
    // Proxy authentication username (CURLOPT_PROXYUSERNAME)
    std::optional<std::string> proxyUserName;
    // Proxy authentication password (CURLOPT_PROXYPASSWORD)
    std::optional<std::string> proxyPassword;
    // Enable curl verbose debug output (CURLOPT_VERBOSE)
    std::optional<bool> enableVerbose;
    // Called with the CURL* handle and RequestMessage before each request is performed,
    // allowing users to set arbitrary curl options via curl_easy_setopt.
    // The void* parameter is the CURL* easy handle.
    std::function<void(void*, const RequestMessage*)> requestInterceptor;
};

} // namespace oss2
} // namespace alibabacloud
