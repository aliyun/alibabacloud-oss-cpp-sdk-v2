
#include "alibabacloud/oss2/retry/ErrorRetryable.h"
#include "alibabacloud/oss2/Error.h"


namespace alibabacloud {
namespace oss2 {

namespace {
static const std::error_code sdk_no_error = make_error_code(SdkErrorCode::NO_ERROR);
}

bool DefaultErrorRetryable::isErrorRetryable(const std::error_code& error) const {
    // Does not belong to oss sdk error.
    if (error.category() != sdk_no_error.category()) {
        return false;
    }

    auto ev = error.value();
    switch (static_cast<SdkErrorCode>(ev)) {
        // ClientError
        case SdkErrorCode::CREDENTIALS_FETCH_ERROR:
        case SdkErrorCode::CRC_INCONSISTENT:
        case SdkErrorCode::CURLE_COULDNT_CONNECT:
        case SdkErrorCode::CURLE_PARTIAL_FILE:
        case SdkErrorCode::CURLE_WRITE_ERROR:
        case SdkErrorCode::CURLE_OPERATION_TIMEDOUT:
        case SdkErrorCode::CURLE_GOT_NOTHING:
        case SdkErrorCode::CURLE_SEND_ERROR:
        case SdkErrorCode::CURLE_RECV_ERROR:
        case SdkErrorCode::CURLE_SEND_FAIL_REWIND:
            return true;

        default:
            // HTTP STATUS CODE
            if ((ev >= 500 && ev < 600) || ev == 401 || ev == 408 || ev == 429) {
                return true;
            }

            // TODO  Service specail error
            // ex. 403-"RequestTimeTooSkewed", 400-"BadRequest"
    }

    return false;
}

} // namespace oss2
} // namespace alibabacloud