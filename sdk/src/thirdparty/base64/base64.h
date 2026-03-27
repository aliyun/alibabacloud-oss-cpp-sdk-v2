
#include <cctype>
#include <utility>

namespace alibabacloud {
namespace oss2 {
namespace thirdparty {

namespace base64 {

char const* get_alphabet();

signed char const* get_inverse();

/// Returns max chars needed to encode a base64 string

std::size_t constexpr encoded_size(std::size_t n) {
    return 4 * ((n + 2) / 3);
}

/// Returns max bytes needed to decode a base64 string
inline std::size_t constexpr decoded_size(std::size_t n) {
    return n / 4 * 3; // requires n&3==0, smaller
}

std::size_t encode(void* dest, void const* src, std::size_t len);


std::pair<std::size_t, std::size_t> decode(void* dest, char const* src, std::size_t len);

} // namespace base64
} // namespace thirdparty
} // namespace oss2
} // namespace alibabacloud
