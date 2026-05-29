#pragma once

#include "src/thirdparty/tinyxml2/tinyxml2.hpp"

#include <cstdint>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace transform {

std::string toXmlText(const std::string& value, const std::string& tag);
std::string toXmlText(bool value, const std::string& tag);
std::string toXmlText(int32_t value, const std::string& tag);
std::string toXmlText(int64_t value, const std::string& tag);
std::string toXmlText(double value, const std::string& tag);

bool toBool(const thirdparty::tinyxml2::XMLElement* node);
std::int32_t toInt32(const thirdparty::tinyxml2::XMLElement* node);
std::int64_t toInt64(const thirdparty::tinyxml2::XMLElement* node);
double toDouble(const thirdparty::tinyxml2::XMLElement* node);
std::string toString(const thirdparty::tinyxml2::XMLElement* node);
std::string toString(const thirdparty::tinyxml2::XMLElement* node, bool doDecode);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud