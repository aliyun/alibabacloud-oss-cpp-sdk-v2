
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <string>
#include <system_error>


namespace alibabacloud {
namespace oss2 {

/// Error codes from library operations
enum class SdkErrorCode {
    NO_ERROR = 0,

    // server: [100-600)
    SERVER_START = 100,
    SERVER_END = 599,

    // client: [100000-199999]
    CLIENT_START = 100000,

    CRC_INCONSISTENT = CLIENT_START + 1,
    REQUEST_DISABLE,

    NULL_POINTER,
    ARGUMENT_INVALID,
    ARGUMENT_NULL,
    ARGUMENT_REQUIRED,
    OPERATION_UNSUPPORT,
    OPERATION_CANCELED,
    ENDPOINT_REGION_NULL,
    ENDPOINT_INVALID,
    REQUEST_METHOD_EMPTY,

    BUCKET_NAME_INVALID,
    OBJECT_NAME_INVALID,

    READ_DATA_FAIL,

    // Credentials
    CREDENTIALS_START = CLIENT_START + 1000,
    CREDENTIALS_EMPTYNULL = CREDENTIALS_START + 1,
    CREDENTIALS_FETCH_ERROR,
    CREDENTIALS_PROVIDER_NULL,

    CREDENTIALS_END = CLIENT_START + 1999,

    // Signer
    SIGNER_START = CLIENT_START + 2000,
    SIGN_ERROR = SIGNER_START + 1,

    SIGNER_END = CLIENT_START + 2999,

    // Serialization & Deserialization
    SERDE_START = CLIENT_START + 3000,
    Deserialization_ERROR,
    SERDE_END = CLIENT_START + 3999,

    CLIENT_END = 199999,

    // transport
    TRANSPORT_START = 200000,

    // transport:curl [200000-200999], 200000 + CURLcode
    CURLE_START = TRANSPORT_START,
    CURLE_COULDNT_CONNECT = CURLE_START + 7,
    CURLE_PARTIAL_FILE = CURLE_START + 18,
    CURLE_WRITE_ERROR = CURLE_START + 23,
    CURLE_OPERATION_TIMEDOUT = CURLE_START + 28,
    CURLE_GOT_NOTHING = CURLE_START + 52,
    CURLE_SEND_ERROR = CURLE_START + 55,
    CURLE_RECV_ERROR = CURLE_START + 56,
    CURLE_SEND_FAIL_REWIND = CURLE_START + 65,

    CURL_END = TRANSPORT_START + 999,
};

std::error_code make_error_code(SdkErrorCode e);

/// Error conditions corresponding to sets of library error codes.
// enum class SdkErrorCondition {};

// std::error_condition make_error_condition(SdkErrorCondition e);

/*
The status comes from the following modules: client, server, httpclient(ex. curl).
server: [100-600)
client: [100000-199999]
curl  : [200000-299999], 200000 + CURLcode

it's sucessful only if the status/100 equals to 2.
*/
const int ERROR_CLIENT_BASE = 100000;
const int ERROR_CRC_INCONSISTENT = ERROR_CLIENT_BASE + 1;
const int ERROR_REQUEST_DISABLE = ERROR_CLIENT_BASE + 2;

const int ERROR_CURL_BASE = 200000;

class ALIBABACLOUD_OSS_API Error {
  public:
    Error() = default;
    Error(const std::string& code, const std::string& message) : status_(0), code_(code), message_(message) {}
    ~Error() = default;

    long status() const {
        return status_;
    }
    const std::string& code() const {
        return code_;
    }
    const std::string& message() const {
        return message_;
    }

  private:
    long status_{0};
    std::string code_;
    std::string message_;
};

} // namespace oss2
} // namespace alibabacloud


namespace std {
template <>
struct is_error_code_enum<::alibabacloud::oss2::SdkErrorCode> : true_type {};

// template <>
// struct is_error_condition_enum<::alibabacloud::oss2::SdkErrorCondition> : true_type {};

} // namespace std
