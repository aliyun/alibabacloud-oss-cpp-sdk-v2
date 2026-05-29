#include "HttpTransportFactory.h"
#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_HAS_CURL
#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"
#endif

#ifdef ALIBABACLOUD_OSS_HAS_WINHTTP
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportFactory.h"
#endif

namespace alibabacloud {
namespace oss2 {
namespace transport {

std::shared_ptr<HttpTransport> HttpTransportFactory::create([[maybe_unused]] const HttpTransportOptions& options) {
#ifdef ALIBABACLOUD_OSS_HAS_CURL
    CurlTransportOptions curlOpts;
    static_cast<HttpTransportOptions&>(curlOpts) = options;
    return CurlTransportFactory::createHttpTransport(curlOpts);
#elif defined(ALIBABACLOUD_OSS_HAS_WINHTTP)
    WinHttpTransportOptions winOpts;
    static_cast<HttpTransportOptions&>(winOpts) = options;
    return WinHttpTransportFactory::createHttpTransport(winOpts);
#else
    return std::make_shared<NopHttpTransport>();
#endif
}

std::shared_ptr<AsyncHttpTransport> AsyncHttpTransportFactory::create(
    [[maybe_unused]] const HttpTransportOptions& options) {
#ifdef ALIBABACLOUD_OSS_HAS_CURL
    CurlTransportOptions curlOpts;
    static_cast<HttpTransportOptions&>(curlOpts) = options;
    return CurlTransportFactory::createAsyncHttpTransport(curlOpts);
#elif defined(ALIBABACLOUD_OSS_HAS_WINHTTP)
    WinHttpTransportOptions winOpts;
    static_cast<HttpTransportOptions&>(winOpts) = options;
    return WinHttpTransportFactory::createAsyncHttpTransport(winOpts);
#else
    return std::make_shared<NopAsyncHttpTransport>();
#endif
}

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
