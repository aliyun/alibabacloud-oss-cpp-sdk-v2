
#include "OSSUtils.h"
#include "src/utils/Utils.h"

#include <regex>
#include <set>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {

bool isValidIp(const std::string& host) {
    static const std::regex ipPattern(
            "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-"
            "9])");
    return std::regex_match(host, ipPattern);
}

bool isValidBucketName(const std::string& bucket) {
    static const std::regex namePattern("^[a-z0-9][a-z0-9\\-]{1,61}[a-z0-9]$");
    if (bucket.empty())
        return false;
    return std::regex_match(bucket, namePattern);
}

bool isValidObjectName(const std::string& key) {
    if (key.empty() || !key.compare(0, 1, "\\", 1))
        return false;

    return key.size() <= 1023;
}

bool isValidMethod(const std::string& key) {
    static const std::set<std::string> methods = {"PUT", "GET", "POST", "HEAD", "DELETE", "OPTIONS"};
    return methods.find(key) != methods.end();
}

std::string addScheme(const std::string& value, bool disableSsl) {
    static const std::regex pattern("^[^:]+://.*");
    if (!std::regex_match(value, pattern)) {
        std::stringstream ss;
        if (disableSsl) {
            ss << "http";
        } else {
            ss << "https";
        }
        ss << "://" << value;
        return ss.str();
    }
    return value;
}

std::string regionToEndpoint(const std::string& value, EndpointType type, bool disableSsl) {
    std::string scheme = disableSsl ? "http" : "https";
    std::string endpoint;
    switch (type) {
        case EndpointType::DualStack:
            endpoint = value + ".oss.aliyuncs.com";
            break;
        case EndpointType::Internal:
            endpoint = "oss-" + value + "-internal.aliyuncs.com";
            break;
        case EndpointType::Accelerate:
            endpoint = "oss-accelerate.aliyuncs.com";
            break;
        case EndpointType::Overseas:
            endpoint = "oss-accelerate-overseas.aliyuncs.com";
            break;
        default:
            endpoint = "oss-" + value + ".aliyuncs.com";
            break;
    }
    return scheme + "://" + endpoint;
}

std::string buildHostPath(const OperationInput& input, const std::string& baseUrl,
                          AddressStyleType addressStyle) {
    std::vector<std::string> paths;
    paths.reserve(2);
    auto host = baseUrl;

    if (input.bucket.has_value()) {
        switch (addressStyle) {
            case AddressStyleType::Path:
                paths.emplace_back(input.bucket.value());
                if (!input.key.has_value()) {
                    paths.emplace_back("");
                }
                break;
            case AddressStyleType::CName:
                break;
            case AddressStyleType::VirtualHosted:
            default:
                host = input.bucket.value() + "." + baseUrl;
                break;
        }
    }

    if (input.key.has_value()) {
        paths.emplace_back(utils::UrlEncodePath(input.key.value()));
    }

    return host + "/" + utils::StringJoin(paths, "/");
}

void updateError(ExecuteContext& context, std::error_code errorCode, const char* code, const char* message) {
    context.errorContext.error = errorCode;
    if (code) {
        context.errorContext.errorFields.emplace("Code", code);
    }
    if (message) {
        context.errorContext.errorFields.emplace("Message", message);
    }
}

void updateError(ExecuteContext& context, std::error_code errorCode, const char* code, std::string&& message) {
    context.errorContext.error = errorCode;
    if (code) {
        context.errorContext.errorFields.emplace("Code", code);
    }
    if (!message.empty()) {
        context.errorContext.errorFields.emplace("Message", message);
    }
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
