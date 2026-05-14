#include "HttpTransportFactory.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"

namespace alibabacloud {
namespace oss2 {
namespace transport {

std::shared_ptr<HttpTransport> HttpTransportFactory::create(const HttpTransportOptions& options) {
    CurlTransportOptions curlOpts;
    static_cast<HttpTransportOptions&>(curlOpts) = options;
    return CurlTransportFactory::createHttpTransport(curlOpts);
}

std::shared_ptr<AsyncHttpTransport> AsyncHttpTransportFactory::create(const HttpTransportOptions& options) {
    CurlTransportOptions curlOpts;
    static_cast<HttpTransportOptions&>(curlOpts) = options;
    return CurlTransportFactory::createAsyncHttpTransport(curlOpts);
}

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
