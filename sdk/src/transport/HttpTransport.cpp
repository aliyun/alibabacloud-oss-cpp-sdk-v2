
#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {

ResponseResult NopHttpTransport::send(std::unique_ptr<RequestMessage>&, RequestContext&) {
    return std::error_code();
}


} // namespace oss2
} // namespace alibabacloud