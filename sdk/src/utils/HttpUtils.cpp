
#include "Utils.h"
#include <sstream>
#include <cctype>


namespace alibabacloud {
namespace oss2 {
namespace utils {

std::string urlEncode(const std::string& src, bool ignoreSlash) {
    std::stringstream dest;
    static const char* hex = "0123456789ABCDEF";
    // unsigned char c;

    for (size_t i = 0; i < src.size(); i++) {
        unsigned char c = src[i];
        if (isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~')) {
            dest << c;
        } else if (c == ' ') {
            dest << "%20";
        } else if (ignoreSlash && c == '/') {
            dest << c;
        } else {
            dest << '%' << hex[c >> 4] << hex[c & 15];
        }
    }

    return dest.str();
}

std::string UrlEncodePath(const std::string& src) {
    return urlEncode(src, true);
}

std::string UrlEncode(const std::string& src) {
    return urlEncode(src, false);
}

std::string UrlDecode(const std::string& src) {
    std::stringstream unescaped;
    unescaped.fill('0');
    unescaped << std::hex;

    size_t safeLength = src.size();
    const char* safe = src.c_str();
    for (auto i = safe, n = safe + safeLength; i != n; ++i) {
        char c = *i;
        if (c == '%') {
            if (i + 2 >= n) {
                unescaped << c;
                break;
            }

            char hex[3];
            hex[0] = *(i + 1);
            hex[1] = *(i + 2);

            if (std::isxdigit(static_cast<unsigned char>(hex[0])) &&
                std::isxdigit(static_cast<unsigned char>(hex[1]))) {
                hex[2] = 0;
                i += 2;
                auto hexAsInteger = strtol(hex, nullptr, 16);
                unescaped << static_cast<char>(hexAsInteger);
            } else {
                unescaped << c;
            }
        } else {
            unescaped << *i;
        }
    }

    return unescaped.str();
}

ParameterCollection ToEncodedParameters(const std::string& url) {
    // find query part
    auto queryPos = url.find("?");
    if (queryPos == std::string::npos) {
        return {};
    }

    // no segment
    auto query = std::string_view(url.data() + queryPos, url.size() - queryPos);

    ParameterCollection parameters;
    // extract query to parameters map
    auto cur = query.begin();
    if (cur != query.end() && *cur == '?') {
        ++cur;
    }

    while (cur != query.end()) {
        auto value_end = std::find(cur, query.end(), '&');
        auto key_end = std::find(cur, value_end, '=');

        std::string query_value;
        std::string query_key;
        if (key_end < value_end) {
            query_value = std::string(key_end + 1, value_end);
            query_key = std::string(cur, key_end);
        } else {
            query_key = std::string(cur, key_end);
        }

        cur = value_end;
        if (cur != query.end()) {
            ++cur;
        }

        parameters.emplace(std::move(query_key), std::move(query_value));
    }

    return parameters;
}

std::string ToQueryString(const ParameterCollection& parameters) {
    std::stringstream ss;
    if (!parameters.empty()) {
        bool first = true;
        for (const auto& [k, v] : parameters) {
            if (!first) {
                ss << "&";
            }
            ss << UrlEncode(k);
            if (!v.empty()) {
                ss << "=" << UrlEncode(v);
            }
            first = false;
        }
    }
    return ss.str();
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud