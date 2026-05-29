
#include "Utils.h"
#include <algorithm>
#include <cstring>
#include <sstream>


namespace alibabacloud {
namespace oss2 {
namespace utils {

void StringReplace(std::string& src, const std::string& s1, const std::string& s2) {
    std::string::size_type pos = 0;
    while ((pos = src.find(s1, pos)) != std::string::npos) {
        src.replace(pos, s1.length(), s2);
        pos += s2.length();
    }
}

std::string LeftTrim(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](unsigned char ch) { return !::isspace(ch); }));
    return copy;
}

std::string RightTrim(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](unsigned char ch) { return !::isspace(ch); }).base(),
               copy.end());
    return copy;
}

std::string Trim(const char* source) {
    return LeftTrim(RightTrim(source).c_str());
}

std::string LeftTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](int ch) { return !(ch == '"'); }));
    return copy;
}

std::string RightTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](int ch) { return !(ch == '"'); }).base(), copy.end());
    return copy;
}

std::string TrimQuotes(const char* source) {
    return LeftTrimQuotes(RightTrimQuotes(source).c_str());
}

std::string ToLower(const char* source) {
    std::string copy;
    if (source) {
        size_t srcLength = strlen(source);
        copy.resize(srcLength);
        std::transform(source, source + srcLength, copy.begin(), [](unsigned char c) { return (char) ::tolower(c); });
    }
    return copy;
}

std::string ToUpper(const char* source) {
    std::string copy;
    if (source) {
        size_t srcLength = strlen(source);
        copy.resize(srcLength);
        std::transform(source, source + srcLength, copy.begin(), [](unsigned char c) { return (char) ::toupper(c); });
    }
    return copy;
}


std::string StringJoin(const std::vector<std::string>& elements, std::string_view delimiter) {
    if (elements.empty()) {
        return {};
    }

    size_t total_len = 0;
    for (const auto& s : elements) {
        total_len += s.length();
    }
    total_len += delimiter.length() * (elements.size() - 1);

    std::string result;
    result.reserve(total_len);

    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) {
            result.append(delimiter);
        }
        result.append(elements[i]);
    }

    return result;
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud