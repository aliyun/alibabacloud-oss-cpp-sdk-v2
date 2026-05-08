
#include "SerdeUtils.h"
#include "src/utils/Utils.h"

#include <charconv>
#include <system_error>

namespace alibabacloud {
namespace oss2 {
namespace transform {

std::string toXmlText(const std::string& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(utils::XmlEscape(value));
    str.append("</").append(tag).append(">");
    return str;
}

std::string toXmlText(bool value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(value ? "true" : "false");
    str.append("</").append(tag).append(">");
    return str;
}

std::string toXmlText(int32_t value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(std::to_string(value));
    str.append("</").append(tag).append(">");
    return str;
}

std::string toXmlText(int64_t value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(std::to_string(value));
    str.append("</").append(tag).append(">");
    return str;
}

std::string toXmlText(double value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(std::to_string(value));
    str.append("</").append(tag).append(">");
    return str;
}

bool toBool(thirdparty::tinyxml2::XMLElement* node) {
    bool value;
    auto err = node->QueryBoolText(&value);
    return err == thirdparty::tinyxml2::XML_SUCCESS ? value : false;
}

int32_t toInt32(thirdparty::tinyxml2::XMLElement* node) {
    int64_t value;
    auto err = node->QueryInt64Text(&value);
    if (err != thirdparty::tinyxml2::XML_SUCCESS) {
        return 0;
    }

    if (value > INT32_MAX) {
        return INT32_MAX;
    }
    return static_cast<int32_t>(value);
    // return node->GetText() ? static_cast<int64_t>(std::atoi(node->GetText())) : 0;
}

int64_t toInt64(thirdparty::tinyxml2::XMLElement* node) {
    int64_t value;
    auto err = node->QueryInt64Text(&value);
    return err == thirdparty::tinyxml2::XML_SUCCESS ? value : 0;
    // return node->GetText() ? static_cast<int64_t>(std::atoll(node->GetText())) : 0;
}

double toDouble(thirdparty::tinyxml2::XMLElement* node) {
    double value;
    auto err = node->QueryDoubleText(&value);
    return err == thirdparty::tinyxml2::XML_SUCCESS ? value : 0.0;
}

std::string toString(thirdparty::tinyxml2::XMLElement* node) {
    return node->GetText() ? node->GetText() : "";
}

std::string toString(thirdparty::tinyxml2::XMLElement* node, bool doDecode) {
    if (node->GetText() == nullptr) {
        return "";
    }
    std::string value = node->GetText();
    return doDecode? utils::UrlDecode(value): value;
}

} // namespace transform
} // namespace oss2
} // namespace alibabacloud