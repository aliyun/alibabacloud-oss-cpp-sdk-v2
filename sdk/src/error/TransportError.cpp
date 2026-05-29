
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class transport_error_category : public std::error_category {
  public:
    const char* name() const noexcept override {
        return "oss2.transport";
    }

    std::string message(int ev) const override {
        switch (static_cast<TransportErrorCode>(ev)) {
            case TransportErrorCode::ConnectionFailed: return "connection failed";
            case TransportErrorCode::DnsError: return "DNS resolution failed";
            case TransportErrorCode::SslError: return "SSL/TLS error";
            case TransportErrorCode::Timeout: return "operation timed out";
            case TransportErrorCode::SendRecvError: return "send/receive error";
            case TransportErrorCode::PartialTransfer: return "partial transfer";
            case TransportErrorCode::Canceled: return "operation canceled";
            case TransportErrorCode::NotSupported: return "operation not supported";
            case TransportErrorCode::Unknown: return "unknown transport error";
            default: return "unknown transport error";
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        auto ec = static_cast<TransportErrorCode>(code);
        if (cond == make_error_condition(ErrorCondition::Retryable)) {
            return ec == TransportErrorCode::ConnectionFailed || ec == TransportErrorCode::DnsError
                || ec == TransportErrorCode::Timeout || ec == TransportErrorCode::SendRecvError
                || ec == TransportErrorCode::PartialTransfer;
        }
        if (cond == make_error_condition(ErrorCondition::Canceled)) {
            return ec == TransportErrorCode::Canceled;
        }
        if (cond == make_error_condition(ErrorCondition::NonRetryable)) {
            return ec == TransportErrorCode::SslError || ec == TransportErrorCode::NotSupported
                || ec == TransportErrorCode::Unknown;
        }
        return false;
    }
};

} // namespace

std::error_code make_error_code(TransportErrorCode e) {
    static const transport_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
