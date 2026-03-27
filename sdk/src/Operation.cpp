
#include "alibabacloud/oss2/Operation.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {

namespace detail {
const static std::string EMPTY;
}

const std::string& OperationError::getCode() const {
    if (errorFields_.find("Code") != errorFields_.end()) {
        return errorFields_.at("Code");
    }
    return detail::EMPTY;
}

const std::string& OperationError::getMessage() const {
    if (errorFields_.find("Message") != errorFields_.end()) {
        return errorFields_.at("Message");
    }
    return detail::EMPTY;
}

const std::string& OperationError::getEC() const {
    if (errorFields_.find("EC") != errorFields_.end()) {
        return errorFields_.at("EC");
    }
    if (headers_.find("x-oss-ec") != headers_.end()) {
        return headers_.at("x-oss-ec");
    }
    return detail::EMPTY;
}

const std::string& OperationError::getRequestId() const {
    if (errorFields_.find("RequestId") != errorFields_.end()) {
        return errorFields_.at("RequestId");
    }
    if (headers_.find("x-oss-request-id") != headers_.end()) {
        return headers_.at("x-oss-request-id");
    }
    return detail::EMPTY;
}

std::string OperationError::toString() const {
    std::stringstream ss;
    ss << "Operation " << opName_ << " fail. Caused by ";
    int ev = errorCode_.value();

    // Service Error
    if (ev >= 400 && ev <= 599) {
        ss << "Error returned by Service." << std::endl;
        ss << "Http Status Code: " << statusCode_ << std::endl;
        ss << "Error Code: " << getCode() << std::endl;
        ss << "Request Id: " << getRequestId() << std::endl;
        ss << "Message: " << getMessage() << std::endl;
        ss << "EC: " << getEC() << std::endl;
        ss << "Timestamp: ";
        if (headers_.find("Date") != headers_.end()) {
            ss << headers_.at("Date") << std::endl;
        }
        ss << "Request Endpoint:" << requestTarget_ << std::endl;
    } else {
        // Client Error
        ss << "Error returned by Client." << std::endl;
        ss << "Error Code: " << getCode() << std::endl;
        ss << "Message: " << getMessage() << std::endl;
    }
    return ss.str();
}

} // namespace oss2
} // namespace alibabacloud