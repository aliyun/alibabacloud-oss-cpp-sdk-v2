#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace transport {

class HttpTransportFactory {
  public:
    static std::shared_ptr<HttpTransport> create(const HttpTransportOptions& options);
};

class AsyncHttpTransportFactory {
  public:
    static std::shared_ptr<AsyncHttpTransport> create(const HttpTransportOptions& options);
};

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
