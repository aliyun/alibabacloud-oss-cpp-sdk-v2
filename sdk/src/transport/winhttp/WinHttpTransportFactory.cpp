#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportFactory.h"
#include "WinHttpClient.h"
#include "WinHttpAsyncClient.h"

namespace alibabacloud {
namespace oss2 {

std::shared_ptr<HttpTransport> WinHttpTransportFactory::createHttpTransport(
        const WinHttpTransportOptions& options) {
    return std::make_shared<transport::winhttp::WinHttpClient>(options);
}

std::shared_ptr<AsyncHttpTransport> WinHttpTransportFactory::createAsyncHttpTransport(
        const WinHttpTransportOptions& options) {
    return std::make_shared<transport::winhttp::WinHttpAsyncClient>(options);
}

} // namespace oss2
} // namespace alibabacloud
