#include "HttpTransportFactory.h"
#include "curl/CurlHttpClient.h"
#include "curl/CurlMultiTransport.h"

namespace alibabacloud {
namespace oss2 {
namespace transport {

std::shared_ptr<HttpTransport> HttpTransportFactory::create(const HttpTransportOptions& options) {
    return std::make_shared<curl::CurlHttpClient>(options);
}

std::shared_ptr<AsyncHttpTransport> AsyncHttpTransportFactory::create(const HttpTransportOptions& options) {
    return std::make_shared<curl::CurlMultiTransport>(options);
}

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
