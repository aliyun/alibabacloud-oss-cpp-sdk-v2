
#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>


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

            if (std::isxdigit(static_cast<unsigned char>(hex[0]))
                && std::isxdigit(static_cast<unsigned char>(hex[1]))) {
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

bool ParseRangeHeader(const std::string& s, std::vector<std::pair<std::int64_t, std::int64_t>>& ranges) {
    constexpr std::string_view kPrefix = "bytes=";
    if (s.size() <= kPrefix.size() || std::string_view(s).substr(0, kPrefix.size()) != kPrefix) {
        return false;
    }

    auto trim = [](std::string_view sv) {
        while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t')) {
            sv.remove_prefix(1);
        }
        while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t')) {
            sv.remove_suffix(1);
        }
        return sv;
    };

    auto parseSide = [](std::string_view sv, std::int64_t& out) -> bool {
        if (sv.empty()) {
            out = -1;
            return true;
        }
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
        return ec == std::errc{} && ptr == sv.data() + sv.size() && out >= 0;
    };

    std::string_view body = std::string_view(s).substr(kPrefix.size());
    while (!body.empty()) {
        auto comma = body.find(',');
        auto item = trim(comma == std::string_view::npos ? body : body.substr(0, comma));

        auto dash = item.find('-');
        if (dash == std::string_view::npos) {
            return false;
        }

        std::int64_t first = -1, last = -1;
        if (!parseSide(item.substr(0, dash), first)) {
            return false;
        }
        if (!parseSide(item.substr(dash + 1), last)) {
            return false;
        }
        if (first == -1 && last == -1) {
            return false;
        }
        if (first != -1 && last != -1 && first > last) {
            return false;
        }

        ranges.emplace_back(first, last);
        if (comma == std::string_view::npos) {
            break;
        }
        body.remove_prefix(comma + 1);
    }
    return !ranges.empty();
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
