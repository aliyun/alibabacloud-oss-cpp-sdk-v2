
#include "HttpTransportFactory.h"

#ifdef USE_WINHTTP
#include "winhttp/WinHttpClient.h"
#else
#include "curl/CurlHttpClient.h"
#endif

namespace alibabacloud {
namespace oss2 {
namespace transport {

std::shared_ptr<HttpTransport> CreateDefaultHttpTransport(const HttpTransportOptions& options) {
#ifdef USE_WINHTTP
    return std::make_shared<winhttp::WinHttpClient>(options);
#else
    return std::make_shared<curl::CurlHttpClient>(options);
#endif
}

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
