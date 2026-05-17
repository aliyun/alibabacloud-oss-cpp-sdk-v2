
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <system_error>

namespace alibabacloud {
namespace oss2 {

// ---------------------------------------------------------------------------
// ErrorCondition - cross-category semantic matching
// ---------------------------------------------------------------------------

enum class ErrorCondition {
    Retryable = 1,
    NonRetryable,
    Canceled,
    InvalidArgument,
    AuthenticationError,
};

ALIBABACLOUD_OSS_API std::error_condition make_error_condition(ErrorCondition e);

// ---------------------------------------------------------------------------
// ClientErrorCode - validation, argument, and general client errors
// ---------------------------------------------------------------------------

enum class ClientErrorCode {
    ArgumentInvalid = 1,
    ArgumentRequired,
    EndpointInvalid,
    EndpointRegionNull,
    BucketNameInvalid,
    ObjectNameInvalid,
    CrcMismatch,
    RequestDisable,
    OperationCanceled,
    OperationNotSupported,
    RequestMethodEmpty,
    ReadDataFail,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(ClientErrorCode e);

// ---------------------------------------------------------------------------
// CredentialsErrorCode - credential retrieval and provider errors
// ---------------------------------------------------------------------------

enum class CredentialsErrorCode {
    Empty = 1,
    FetchError,
    ProviderNull,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(CredentialsErrorCode e);

// ---------------------------------------------------------------------------
// SignerErrorCode - request signing errors
// ---------------------------------------------------------------------------

enum class SignerErrorCode {
    SignFailed = 1,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(SignerErrorCode e);

// ---------------------------------------------------------------------------
// SerdeErrorCode - serialization/deserialization errors
// ---------------------------------------------------------------------------

enum class SerdeErrorCode {
    DeserializationFailed = 1,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(SerdeErrorCode e);

// ---------------------------------------------------------------------------
// ServerError - HTTP status codes as error codes (no enum, int = status)
// ---------------------------------------------------------------------------

ALIBABACLOUD_OSS_API std::error_code make_server_error_code(int httpStatus);
ALIBABACLOUD_OSS_API std::error_code make_retryable_server_error_code(int httpStatus);

// ---------------------------------------------------------------------------
// TransportErrorCode - abstract transport-layer errors
// ---------------------------------------------------------------------------

enum class TransportErrorCode {
    ConnectionFailed = 1,
    DnsError,
    SslError,
    Timeout,
    SendRecvError,
    PartialTransfer,
    Canceled,
    NotSupported,
    Unknown,
};

ALIBABACLOUD_OSS_API std::error_code make_error_code(TransportErrorCode e);


} // namespace oss2
} // namespace alibabacloud

template<>
struct std::is_error_condition_enum<alibabacloud::oss2::ErrorCondition> : std::true_type {};

template<>
struct std::is_error_code_enum<alibabacloud::oss2::ClientErrorCode> : std::true_type {};

template<>
struct std::is_error_code_enum<alibabacloud::oss2::CredentialsErrorCode> : std::true_type {};

template<>
struct std::is_error_code_enum<alibabacloud::oss2::SignerErrorCode> : std::true_type {};

template<>
struct std::is_error_code_enum<alibabacloud::oss2::SerdeErrorCode> : std::true_type {};

template<>
struct std::is_error_code_enum<alibabacloud::oss2::TransportErrorCode> : std::true_type {};
