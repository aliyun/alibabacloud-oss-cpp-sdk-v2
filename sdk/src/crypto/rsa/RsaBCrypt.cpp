#include "../RsaUtils.h"

#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <vector>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

namespace alibabacloud {
namespace oss2 {
namespace crypto {

static bool pemToDer(const std::string& pem, std::vector<BYTE>& der) {
    DWORD derLen = 0;
    if (!CryptStringToBinaryA(pem.data(), (DWORD)pem.size(),
                              CRYPT_STRING_BASE64HEADER,
                              nullptr, &derLen, nullptr, nullptr)) {
        return false;
    }
    der.resize(derLen);
    if (!CryptStringToBinaryA(pem.data(), (DWORD)pem.size(),
                              CRYPT_STRING_BASE64HEADER,
                              der.data(), &derLen, nullptr, nullptr)) {
        return false;
    }
    der.resize(derLen);
    return true;
}

static BCRYPT_KEY_HANDLE importPublicKey(const std::string& publicKeyPem) {
    std::vector<BYTE> der;
    if (!pemToDer(publicKeyPem, der)) return nullptr;

    CERT_PUBLIC_KEY_INFO* pubKeyInfo = nullptr;
    DWORD pubKeyInfoLen = 0;
    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO,
                             der.data(), (DWORD)der.size(),
                             CRYPT_DECODE_ALLOC_FLAG, nullptr,
                             &pubKeyInfo, &pubKeyInfoLen)) {
        return nullptr;
    }

    BCRYPT_KEY_HANDLE hKey = nullptr;
    if (!CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, pubKeyInfo, 0, nullptr, &hKey)) {
        LocalFree(pubKeyInfo);
        return nullptr;
    }
    LocalFree(pubKeyInfo);
    return hKey;
}

static BCRYPT_KEY_HANDLE importRsaBlob(BYTE* rsaBlob, DWORD rsaBlobLen) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0);
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status = BCryptImportKeyPair(hAlg, nullptr, BCRYPT_RSAPRIVATE_BLOB,
                                          &hKey, rsaBlob, rsaBlobLen, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return BCRYPT_SUCCESS(status) ? hKey : nullptr;
}

static BCRYPT_KEY_HANDLE importPrivateKey(const std::string& privateKeyPem) {
    std::vector<BYTE> der;
    if (!pemToDer(privateKeyPem, der)) return nullptr;

    // Try PKCS#8 wrapper first
    CRYPT_PRIVATE_KEY_INFO* pkcs8Info = nullptr;
    DWORD pkcs8Len = 0;
    if (CryptDecodeObjectEx(X509_ASN_ENCODING, PKCS_PRIVATE_KEY_INFO,
                            der.data(), (DWORD)der.size(),
                            CRYPT_DECODE_ALLOC_FLAG, nullptr,
                            &pkcs8Info, &pkcs8Len)) {
        BYTE* rsaBlob = nullptr;
        DWORD rsaBlobLen = 0;
        BOOL ok = CryptDecodeObjectEx(X509_ASN_ENCODING, CNG_RSA_PRIVATE_KEY_BLOB,
                                      pkcs8Info->PrivateKey.pbData, pkcs8Info->PrivateKey.cbData,
                                      CRYPT_DECODE_ALLOC_FLAG, nullptr,
                                      &rsaBlob, &rsaBlobLen);
        LocalFree(pkcs8Info);
        if (!ok) return nullptr;
        auto hKey = importRsaBlob(rsaBlob, rsaBlobLen);
        LocalFree(rsaBlob);
        return hKey;
    }

    // Fallback: try raw RSA private key DER
    BYTE* rsaBlob = nullptr;
    DWORD rsaBlobLen = 0;
    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, CNG_RSA_PRIVATE_KEY_BLOB,
                             der.data(), (DWORD)der.size(),
                             CRYPT_DECODE_ALLOC_FLAG, nullptr,
                             &rsaBlob, &rsaBlobLen)) {
        return nullptr;
    }
    auto hKey = importRsaBlob(rsaBlob, rsaBlobLen);
    LocalFree(rsaBlob);
    return hKey;
}

std::string RsaPublicEncrypt(const std::string& publicKeyPem, const std::string& plaintext) {
    BCRYPT_KEY_HANDLE hKey = importPublicKey(publicKeyPem);
    if (!hKey) return {};

    ULONG outLen = 0;
    NTSTATUS status = BCryptEncrypt(hKey,
                                    (PUCHAR)plaintext.data(), (ULONG)plaintext.size(),
                                    nullptr, nullptr, 0,
                                    nullptr, 0, &outLen,
                                    BCRYPT_PAD_PKCS1);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        return {};
    }

    std::string result(outLen, '\0');
    status = BCryptEncrypt(hKey,
                           (PUCHAR)plaintext.data(), (ULONG)plaintext.size(),
                           nullptr, nullptr, 0,
                           (PUCHAR)result.data(), outLen, &outLen,
                           BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey);
    if (!BCRYPT_SUCCESS(status)) return {};
    result.resize(outLen);
    return result;
}

std::string RsaPrivateDecrypt(const std::string& privateKeyPem, const std::string& ciphertext) {
    BCRYPT_KEY_HANDLE hKey = importPrivateKey(privateKeyPem);
    if (!hKey) return {};

    ULONG outLen = 0;
    NTSTATUS status = BCryptDecrypt(hKey,
                                    (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(),
                                    nullptr, nullptr, 0,
                                    nullptr, 0, &outLen,
                                    BCRYPT_PAD_PKCS1);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        return {};
    }

    std::string result(outLen, '\0');
    status = BCryptDecrypt(hKey,
                           (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(),
                           nullptr, nullptr, 0,
                           (PUCHAR)result.data(), outLen, &outLen,
                           BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey);
    if (!BCRYPT_SUCCESS(status)) return {};
    result.resize(outLen);
    return result;
}

} // namespace crypto
} // namespace oss2
} // namespace alibabacloud
