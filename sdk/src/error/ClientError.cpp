
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class client_error_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.client"; }

    std::string message(int ev) const override {
        switch (static_cast<ClientErrorCode>(ev)) {
            case ClientErrorCode::ArgumentInvalid:
                return "invalid argument";
            case ClientErrorCode::ArgumentRequired:
                return "required argument is missing";
            case ClientErrorCode::EndpointInvalid:
                return "endpoint is invalid";
            case ClientErrorCode::EndpointRegionNull:
                return "endpoint region is null";
            case ClientErrorCode::BucketNameInvalid:
                return "bucket name is invalid";
            case ClientErrorCode::ObjectNameInvalid:
                return "object name is invalid";
            case ClientErrorCode::CrcMismatch:
                return "CRC mismatch";
            case ClientErrorCode::RequestDisable:
                return "request is disabled";
            case ClientErrorCode::OperationCanceled:
                return "operation canceled";
            case ClientErrorCode::OperationNotSupported:
                return "operation not supported";
            case ClientErrorCode::RequestMethodEmpty:
                return "request method is empty";
            case ClientErrorCode::ReadDataFail:
                return "failed to read data";
            case ClientErrorCode::EncryptionFailure:
                return "encryption or decryption failed";
            default:
                return "unknown client error";
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        auto ec = static_cast<ClientErrorCode>(code);
        if (cond == make_error_condition(ErrorCondition::InvalidArgument)) {
            return ec == ClientErrorCode::ArgumentInvalid
                || ec == ClientErrorCode::ArgumentRequired
                || ec == ClientErrorCode::EndpointInvalid
                || ec == ClientErrorCode::EndpointRegionNull
                || ec == ClientErrorCode::BucketNameInvalid
                || ec == ClientErrorCode::ObjectNameInvalid
                || ec == ClientErrorCode::RequestMethodEmpty;
        }
        if (cond == make_error_condition(ErrorCondition::Retryable)) {
            return ec == ClientErrorCode::CrcMismatch;
        }
        if (cond == make_error_condition(ErrorCondition::Canceled)) {
            return ec == ClientErrorCode::OperationCanceled;
        }
        if (cond == make_error_condition(ErrorCondition::NonRetryable)) {
            return ec == ClientErrorCode::RequestDisable
                || ec == ClientErrorCode::OperationNotSupported
                || ec == ClientErrorCode::ReadDataFail
                || ec == ClientErrorCode::EncryptionFailure;
        }
        return false;
    }
};

} // namespace

std::error_code make_error_code(ClientErrorCode e) {
    static const client_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
