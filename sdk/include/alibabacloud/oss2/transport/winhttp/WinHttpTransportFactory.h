#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportOptions.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API WinHttpTransportFactory {
  public:
    static std::shared_ptr<HttpTransport> createHttpTransport(const WinHttpTransportOptions& options);
    static std::shared_ptr<AsyncHttpTransport> createAsyncHttpTransport(const WinHttpTransportOptions& options);
};

} // namespace oss2
} // namespace alibabacloud
