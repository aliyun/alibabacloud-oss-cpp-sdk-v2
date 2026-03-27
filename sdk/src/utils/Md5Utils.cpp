
#include "Utils.h"
#include "src/thirdparty/hash/md5.h"

namespace alibabacloud {
namespace oss2 {
namespace utils {


std::string CalcContentMD5(const std::string& data) {
    return CalcContentMD5(data.data(), data.size());
}

std::string CalcContentMD5(const char* data, size_t size) {
    using namespace thirdparty::hash;
    unsigned char buffer[MD5::HashBytes];
    MD5 md5;
    md5.add(data, size);
    md5.getHash(buffer);
    return Base64Encode(reinterpret_cast<const std::byte*>(buffer), MD5::HashBytes);
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud