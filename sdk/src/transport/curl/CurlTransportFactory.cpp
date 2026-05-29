#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"
#include "CurlHttpClient.h"
#include "CurlMultiTransport.h"

namespace alibabacloud {
namespace oss2 {

std::shared_ptr<HttpTransport> CurlTransportFactory::createHttpTransport(const CurlTransportOptions& options) {
    return std::make_shared<transport::curl::CurlHttpClient>(options);
}

std::shared_ptr<AsyncHttpTransport> CurlTransportFactory::createAsyncHttpTransport(
    const CurlTransportOptions& options) {
    return std::make_shared<transport::curl::CurlMultiTransport>(options);
}

} // namespace oss2
} // namespace alibabacloud
