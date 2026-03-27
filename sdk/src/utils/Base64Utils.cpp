
#include "Utils.h"
#include <algorithm>

#include "src/thirdparty/base64/base64.h"

namespace alibabacloud {
namespace oss2 {
namespace utils {

std::string Base64Encode(const std::string& src) {
    return Base64Encode(reinterpret_cast<const std::byte*>(src.data()), src.size());
}

std::string Base64Encode(const std::byte* src, std::size_t len) {
    std::string out;
    out.resize(thirdparty::base64::encoded_size(len));
    out.resize(thirdparty::base64::encode(&out[0], reinterpret_cast<const void*>(src), len));
    return out;
}

std::string Base64EncodeUrlSafe(const std::string& src) {
    return Base64EncodeUrlSafe(reinterpret_cast<const std::byte*>(src.data()), src.size());
}

std::string Base64EncodeUrlSafe(const std::byte* src, std::size_t len) {
    std::string out;
    out.resize(thirdparty::base64::encoded_size(len));
    out.resize(thirdparty::base64::encode(&out[0], reinterpret_cast<const void*>(src), len));

    while (out.size() > 0 && *out.rbegin() == '=')
        out.pop_back();

    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        if (c == '+')
            return '-';
        if (c == '/')
            return '_';
        return (char) c;
    });
    return out;
}

std::string Base64Decode(const std::string& src) {
    std::string out;
    out.resize(thirdparty::base64::decoded_size(src.size() + 2)); // without two = chars
    auto const result = thirdparty::base64::decode(&out[0], src.data(), src.size());
    out.resize(result.first);
    return out;
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud