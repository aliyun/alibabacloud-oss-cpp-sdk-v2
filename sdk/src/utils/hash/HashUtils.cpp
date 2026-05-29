
#include "../Utils.h"
#include "src/thirdparty/hash/sha1.h"
#include "src/thirdparty/hash/sha256.h"
#include <cstring>


namespace alibabacloud {
namespace oss2 {
namespace utils {

void HmacSha1(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[20]) {
    using namespace thirdparty::hash;

    unsigned char usedKey[SHA1::BlockSize] = {0};

    if (numKeyBytes <= SHA1::BlockSize) {
        memcpy(usedKey, key, numKeyBytes);
    } else {
        SHA1 keyHasher;
        keyHasher.add(key, numKeyBytes);
        keyHasher.getHash(usedKey);
    }

    for (size_t i = 0; i < SHA1::BlockSize; i++) {
        usedKey[i] ^= 0x36;
    }

    unsigned char inside[SHA1::HashBytes];
    SHA1 insideHasher;
    insideHasher.add(usedKey, SHA1::BlockSize);
    insideHasher.add(data, numDataBytes);
    insideHasher.getHash(inside);

    for (size_t i = 0; i < SHA1::BlockSize; i++) {
        usedKey[i] ^= 0x5C ^ 0x36;
    }

    SHA1 finalHasher;
    finalHasher.add(usedKey, SHA1::BlockSize);
    finalHasher.add(inside, SHA1::HashBytes);

    finalHasher.getHash(out);
}

void HmacSh256(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[32]) {
    // initialize key with zeros
    using namespace thirdparty::hash;

    unsigned char usedKey[SHA256::BlockSize] = {0};

    // adjust length of key: must contain exactly blockSize bytes
    if (numKeyBytes <= SHA256::BlockSize) {
        // copy key
        memcpy(usedKey, key, numKeyBytes);
    } else {
        // shorten key: usedKey = hashed(key)
        SHA256 keyHasher;
        keyHasher.add(key, numKeyBytes);
        keyHasher.getHash(usedKey);
    }

    // create initial XOR padding
    for (size_t i = 0; i < SHA256::BlockSize; i++) {
        usedKey[i] ^= 0x36;
    }

    // inside = hash((usedKey ^ 0x36) + data)
    unsigned char inside[SHA256::HashBytes];
    SHA256 insideHasher;
    insideHasher.add(usedKey, SHA256::BlockSize);
    insideHasher.add(data, numDataBytes);
    insideHasher.getHash(inside);

    // undo usedKey's previous 0x36 XORing and apply a XOR by 0x5C
    for (size_t i = 0; i < SHA256::BlockSize; i++) {
        usedKey[i] ^= 0x5C ^ 0x36;
    }

    // hash((usedKey ^ 0x5C) + hash((usedKey ^ 0x36) + data))
    SHA256 finalHasher;
    finalHasher.add(usedKey, SHA256::BlockSize);
    finalHasher.add(inside, SHA256::HashBytes);

    finalHasher.getHash(out);
}

std::string HashSh256(const void* data, size_t numDataBytes) {
    using namespace thirdparty::hash;
    SHA256 sha256;
    return sha256(data, numDataBytes);
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud