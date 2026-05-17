
#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {

ResponseResult NopHttpTransport::send(std::unique_ptr<RequestMessage>&, const RequestOptions&) {
    return TransportError{};
}


} // namespace oss2
} // namespace alibabacloud