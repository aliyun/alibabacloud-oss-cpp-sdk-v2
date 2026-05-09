
#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace transport {

std::shared_ptr<HttpTransport> CreateDefaultHttpTransport(const HttpTransportOptions& options);

} // namespace transport
} // namespace oss2
} // namespace alibabacloud
