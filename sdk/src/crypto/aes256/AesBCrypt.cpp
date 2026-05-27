#include "../Aes256Utils.h"

#include <windows.h>
#include <bcrypt.h>
#include <cstring>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace alibabacloud {
namespace oss2 {
namespace crypto {

struct BcryptAesCtrContext {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    std::vector<UCHAR> keyObj;
    uint8_t counter[16]{};
    uint8_t keystreamBlock[16]{};
    int keystreamOffset = 16;

    ~BcryptAesCtrContext() {
        if (hKey) BCryptDestroyKey(hKey);
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    }
};

AesCtrCipher::AesCtrCipher(const std::string& key, const std::string& iv) : ctx_(nullptr) {
    if ((key.size() != 16 && key.size() != 32) || iv.size() < 16) return;

    auto* ctx = new (std::nothrow) BcryptAesCtrContext;
    if (!ctx) return;

    NTSTATUS status = BCryptOpenAlgorithmProvider(&ctx->hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) { delete ctx; return; }

    status = BCryptSetProperty(ctx->hAlg, BCRYPT_CHAINING_MODE,
                               (PUCHAR)BCRYPT_CHAIN_MODE_ECB,
                               sizeof(BCRYPT_CHAIN_MODE_ECB), 0);
    if (!BCRYPT_SUCCESS(status)) { delete ctx; return; }

    ULONG keyObjLen = 0;
    ULONG cbData = 0;
    status = BCryptGetProperty(ctx->hAlg, BCRYPT_OBJECT_LENGTH,
                               (PUCHAR)&keyObjLen, sizeof(keyObjLen), &cbData, 0);
    if (!BCRYPT_SUCCESS(status)) { delete ctx; return; }

    ctx->keyObj.resize(keyObjLen);
    status = BCryptGenerateSymmetricKey(ctx->hAlg, &ctx->hKey,
                                        ctx->keyObj.data(), keyObjLen,
                                        (PUCHAR)key.data(), (ULONG)key.size(), 0);
    if (!BCRYPT_SUCCESS(status)) { delete ctx; return; }

    std::memcpy(ctx->counter, iv.data(), 16);
    ctx->keystreamOffset = 16;

    ctx_ = ctx;
}

AesCtrCipher::~AesCtrCipher() {
    if (ctx_) {
        delete static_cast<BcryptAesCtrContext*>(ctx_);
    }
}

static void incrementCounter(uint8_t counter[16]) {
    for (int i = 15; i >= 0; --i) {
        if (++counter[i] != 0) break;
    }
}

size_t AesCtrCipher::process(const uint8_t* in, uint8_t* out, size_t len) {
    if (!ctx_) return 0;
    auto* ctx = static_cast<BcryptAesCtrContext*>(ctx_);
    size_t processed = 0;

    while (processed < len) {
        if (ctx->keystreamOffset >= 16) {
            ULONG cbResult = 0;
            NTSTATUS status = BCryptEncrypt(ctx->hKey,
                                           ctx->counter, 16,
                                           nullptr, nullptr, 0,
                                           ctx->keystreamBlock, 16,
                                           &cbResult, 0);
            if (!BCRYPT_SUCCESS(status)) break;
            incrementCounter(ctx->counter);
            ctx->keystreamOffset = 0;
        }

        size_t available = 16 - static_cast<size_t>(ctx->keystreamOffset);
        size_t toProcess = len - processed;
        if (toProcess > available) toProcess = available;

        for (size_t i = 0; i < toProcess; ++i) {
            out[processed + i] = in[processed + i] ^ ctx->keystreamBlock[ctx->keystreamOffset + i];
        }

        ctx->keystreamOffset += static_cast<int>(toProcess);
        processed += toProcess;
    }

    return processed;
}

bool RandomBytes(unsigned char* buf, size_t len) {
    NTSTATUS status = BCryptGenRandom(nullptr, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return BCRYPT_SUCCESS(status);
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
