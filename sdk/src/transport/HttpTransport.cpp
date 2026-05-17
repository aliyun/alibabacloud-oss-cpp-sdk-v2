
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/Error.h"

#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {

ResponseResult NopHttpTransport::send(std::unique_ptr<RequestMessage>&, const RequestOptions&) {
    return TransportError{make_error_code(TransportErrorCode::NotSupported)};
}


} // namespace oss2
} // namespace alibabacloud