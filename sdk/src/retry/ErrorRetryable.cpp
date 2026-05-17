
#include "alibabacloud/oss2/retry/ErrorRetryable.h"
#include "alibabacloud/oss2/Error.h"


namespace alibabacloud {
namespace oss2 {

bool DefaultErrorRetryable::isErrorRetryable(const std::error_code& error) const {
    return error == ErrorCondition::Retryable;
}

} // namespace oss2
} // namespace alibabacloud
