
#include "alibabacloud/oss2/Error.h"

#include <string>

namespace alibabacloud {
namespace oss2 {

class server_error_category : public std::error_category {
  public:
    const char* name() const noexcept override {
        return "oss2.server";
    }

    std::string message(int ev) const override {
        int status = ev >= 10000 ? ev - 10000 : ev;
        switch (status) {
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 408: return "Request Timeout";
            case 409: return "Conflict";
            case 429: return "Too Many Requests";
            case 500: return "Internal Server Error";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";
            default:
                if (status >= 500 && status < 600) {
                    return "Server Error";
                }
                if (status >= 400 && status < 500) {
                    return "Client Error";
                }
                return "HTTP " + std::to_string(status);
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        if (code >= 10000) {
            return cond == make_error_condition(ErrorCondition::Retryable);
        }
        if (cond == make_error_condition(ErrorCondition::Retryable)) {
            return (code >= 500 && code < 600) || code == 401 || code == 408 || code == 429;
        }
        if (cond == make_error_condition(ErrorCondition::NonRetryable)) {
            return code >= 400 && code < 500 && code != 401 && code != 408 && code != 429;
        }
        return false;
    }
};

static const server_error_category g_server_error_category{};

std::error_code make_server_error_code(int httpStatus) {
    return {httpStatus, g_server_error_category};
}

std::error_code make_retryable_server_error_code(int httpStatus) {
    return {10000 + httpStatus, g_server_error_category};
}

} // namespace oss2
} // namespace alibabacloud
