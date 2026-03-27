#pragma once

#include "Defaults.h"
#include "ExecuteMiddleware.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/thirdparty/tinyxml2/tinyxml2.hpp"
#include "src/utils/Utils.h"


#include <regex>
#include <set>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {

enum class EndpointType { Default, DualStack, Internal, Accelerate, Overseas };


static bool isValidIp(const std::string& host) {
    static const std::regex ipPattern(
            "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-"
            "9])");
    return std::regex_match(host, ipPattern);
}

static bool isValidBucketName(const std::string& bucket) {
    static const std::regex namePattern("^[a-z0-9][a-z0-9\\-]{1,61}[a-z0-9]$");
    if (bucket.empty())
        return false;
    return std::regex_match(bucket, namePattern);
}

static bool isValidObjectName(const std::string& key) {
    if (key.empty() || !key.compare(0, 1, "\\", 1))
        return false;

    return key.size() <= 1023;
}

static bool isValidMethod(const std::string& key) {
    static const std::set<std::string> methods = {"PUT", "GET", "POST", "HEAD", "DELETE", "OPTIONS"};
    return methods.find(key) != methods.end();
}

/*
 * Adds a scheme (http/https) to the given URL string if not already present
 */
inline static std::string addScheme(const std::string& value, bool disableSsl) {
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

/*
 * Generates an OSS endpoint based on the region and endpoint type
 */
inline static std::string regionToEndpoint(const std::string& value, EndpointType type, bool disableSsl) {
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

/*
 * Builds the host and path portion based on the provided address style
 */
inline static std::string buildHostPath(const OperationInput& input, const std::string& baseUrl,
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

static void updateError(ExecuteContext& context, SdkErrorCode errorCode, const char* code, const char* message) {
    context.errorContext.error = make_error_code(errorCode);
    if (code) {
        context.errorContext.errorFields.emplace("Code", code);
    }
    if (message) {
        context.errorContext.errorFields.emplace("Message", message);
    }
}

static void updateError(ExecuteContext& context, SdkErrorCode errorCode, const char* code, std::string&& message) {
    context.errorContext.error = make_error_code(errorCode);
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