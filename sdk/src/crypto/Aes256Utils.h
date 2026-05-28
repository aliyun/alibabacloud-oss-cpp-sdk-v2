#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace crypto {

class AesCtrCipher {
public:
    AesCtrCipher(const std::string& key, const std::string& iv);
    ~AesCtrCipher();

    AesCtrCipher(const AesCtrCipher&) = delete;
    AesCtrCipher& operator=(const AesCtrCipher&) = delete;

    size_t process(const uint8_t* in, uint8_t* out, size_t len);

private:
    void* ctx_;
};

bool RandomBytes(unsigned char* buf, size_t len);

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
