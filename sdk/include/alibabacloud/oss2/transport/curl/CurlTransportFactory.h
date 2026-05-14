#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportOptions.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API CurlTransportFactory {
  public:
    static std::shared_ptr<HttpTransport> createHttpTransport(
        const CurlTransportOptions& options);
    static std::shared_ptr<AsyncHttpTransport> createAsyncHttpTransport(
        const CurlTransportOptions& options);
};

} // namespace oss2
} // namespace alibabacloud
