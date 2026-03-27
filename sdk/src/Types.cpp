
#include "alibabacloud/oss2/Types.h"

#include <cstring>

namespace alibabacloud {
namespace oss2 {

namespace detail_ {
static const std::string EMPTY = "";
}

const std::string& RequestModel::getHeaderOrEmpty(const std::string& key) const {
    if (headers_.find(key) != headers_.end()) {
        return headers_.at(key);
    }
    return detail_::EMPTY;
}

const std::string& RequestModel::getParameterOrEmpty(const std::string& key) const {
    if (parameters_.find(key) != parameters_.end()) {
        return parameters_.at(key);
    }
    return detail_::EMPTY;
}

std::int32_t RequestModel::getHeaderAsInt32Or(const std::string& key, std::int32_t value) const {
    if (headers_.find(key) != headers_.end()) {
        return static_cast<std::int32_t>(std::atoi(headers_.at(key).c_str()));
    }
    return value;
}
std::int64_t RequestModel::getHeaderAsInt64Or(const std::string& key, std::int64_t value) const {
    if (headers_.find(key) != headers_.end()) {
        return static_cast<std::int64_t>(std::strtoll(headers_.at(key).c_str(), nullptr, 10));
    }
    return value;
}

bool RequestModel::getHeaderAsBoolOr(const std::string& key, bool value) const {
    if (headers_.find(key) != headers_.end()) {
        return std::strncmp(headers_.at(key).c_str(), "true", 4) == 0;
    }
    return value;
}

std::int32_t RequestModel::getParameterAsInt32Or(const std::string& key, std::int32_t value) const {
    if (parameters_.find(key) != parameters_.end()) {
        return static_cast<std::int32_t>(std::atoi(parameters_.at(key).c_str()));
    }
    return value;
}
std::int64_t RequestModel::getParameterAsInt64Or(const std::string& key, std::int64_t value) const {
    if (parameters_.find(key) != parameters_.end()) {
        return static_cast<std::int64_t>(std::strtoll(parameters_.at(key).c_str(), nullptr, 10));
    }
    return value;
}

bool RequestModel::getParameterAsBoolOr(const std::string& key, bool value) const {
    if (parameters_.find(key) != parameters_.end()) {
        return std::strncmp(parameters_.at(key).c_str(), "true", 4) == 0;
    }
    return value;
}

const std::string& ResultModel::getHeaderOrEmpty(const std::string& key) const {
    if (headers_.find(key) != headers_.end()) {
        return headers_.at(key);
    }
    return detail_::EMPTY;
}

std::int64_t ResultModel::getHeaderAsInt64Or(const std::string& key, std::int64_t value) const {
    if (headers_.find(key) != headers_.end()) {
        return static_cast<std::int64_t>(std::strtoll(headers_.at(key).c_str(), nullptr, 10));
    }
    return value;
}

} // namespace oss2
} // namespace alibabacloud