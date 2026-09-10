
#include "Utils.h"
#include <cstdint>
#include <sstream>
#include <utility>


namespace alibabacloud {
namespace oss2 {
namespace utils {


std::string XmlEscape(const std::string& value) {
    struct Entity {
        const char* pattern;
        char value;
    };

    static const Entity entities[] = {{"&quot;", '\"'}, {"&amp;", '&'}, {"&apos;", '\''},
                                      {"&lt;", '<'},    {"&gt;", '>'},  {"&#13;", '\r'}};

    if (value.empty()) {
        return value;
    }

    std::stringstream ss;
    for (size_t i = 0; i < value.size(); i++) {
        bool flag = false;
        for (size_t j = 0; j < (sizeof(entities) / sizeof(entities[0])); j++) {
            if (value[i] == entities[j].value) {
                flag = true;
                ss << entities[j].pattern;
                break;
            }
        }

        if (!flag) {
            ss << value[i];
        }
    }

    return ss.str();
}

namespace {

// Decode one UTF-8 rune starting at index i. On invalid encoding returns
// (0xFFFD, 1), matching Go's utf8.DecodeRuneInString.
std::pair<uint32_t, int> decodeRune(const std::string& s, size_t i) {
    size_t n = s.size();
    unsigned char b0 = static_cast<unsigned char>(s[i]);
    if (b0 < 0x80) {
        return {b0, 1};
    }
    auto cont = [&](size_t k) -> bool {
        return k < n && (static_cast<unsigned char>(s[k]) & 0xC0) == 0x80;
    };
    if (b0 >= 0xC2 && b0 <= 0xDF) {
        if (cont(i + 1)) {
            uint32_t r = ((b0 & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu);
            return {r, 2};
        }
    } else if (b0 >= 0xE0 && b0 <= 0xEF) {
        if (cont(i + 1) && cont(i + 2)) {
            uint32_t r = ((b0 & 0x0Fu) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 6) |
                         (static_cast<unsigned char>(s[i + 2]) & 0x3Fu);
            if (r >= 0x800 && !(r >= 0xD800 && r <= 0xDFFF)) {
                return {r, 3};
            }
        }
    } else if (b0 >= 0xF0 && b0 <= 0xF4) {
        if (cont(i + 1) && cont(i + 2) && cont(i + 3)) {
            uint32_t r = ((b0 & 0x07u) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 12) |
                         ((static_cast<unsigned char>(s[i + 2]) & 0x3Fu) << 6) |
                         (static_cast<unsigned char>(s[i + 3]) & 0x3Fu);
            if (r >= 0x10000 && r <= 0x10FFFF) {
                return {r, 4};
            }
        }
    }
    return {0xFFFDu, 1};
}

// XML 1.0 legal character range.
bool isInCharacterRange(uint32_t r) {
    return r == 0x09 || r == 0x0A || r == 0x0D || (r >= 0x20 && r <= 0xD7FF) || (r >= 0xE000 && r <= 0xFFFD) ||
           (r >= 0x10000 && r <= 0x10FFFF);
}

} // namespace

// XmlEscapeKey escapes an object key for embedding in XML, additionally
// handling code points outside the XML 1.0 character set that would make the
// server reject the request. Control characters below 0x20 become numeric
// character references; other illegal code points are replaced with U+FFFD.
std::string XmlEscapeKey(const std::string& value) {
    static const char hextable[] = "0123456789ABCDEF";

    std::string out;
    out.reserve(value.size());

    size_t i = 0;
    size_t n = value.size();
    while (i < n) {
        auto [r, width] = decodeRune(value, i);
        const char* esc = nullptr;
        char pattern[7];
        switch (r) {
        case '"':
            esc = "&#34;";
            break;
        case '\'':
            esc = "&#39;";
            break;
        case '&':
            esc = "&amp;";
            break;
        case '<':
            esc = "&lt;";
            break;
        case '>':
            esc = "&gt;";
            break;
        case '\t':
            esc = "&#x9;";
            break;
        case '\n':
            esc = "&#xA;";
            break;
        case '\r':
            esc = "&#xD;";
            break;
        default:
            if (!isInCharacterRange(r) || (r == 0xFFFDu && width == 1)) {
                if (r < 0x20u) {
                    pattern[0] = '&';
                    pattern[1] = '#';
                    pattern[2] = 'x';
                    pattern[3] = hextable[(r >> 4) & 0x0f];
                    pattern[4] = hextable[r & 0x0f];
                    pattern[5] = ';';
                    pattern[6] = '\0';
                    esc = pattern;
                } else {
                    esc = "\xEF\xBF\xBD"; // U+FFFD
                }
            }
            break;
        }

        if (esc != nullptr) {
            out.append(esc);
        } else {
            out.append(value, i, width);
        }
        i += width;
    }

    return out;
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud