#include <gtest/gtest.h>

#include "src/crypto/Aes256Utils.h"
#include "src/crypto/AesCtrContentCipher.h"
#include "src/crypto/AesCtrCipherBuilder.h"
#include "alibabacloud/oss2/crypto/MasterCipher.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <cstring>
#include <sstream>

namespace alibabacloud::oss2 {

namespace {

const char* kKey32 = "12345678901234561234567890123456";
const char* kIV16  = "1234567890123456";
const char* kData16 = "1234567890123456";
const char* kData32 = "12345678901234561234567890123456";

class MockMasterCipher : public crypto::MasterCipher {
  public:
    std::string encrypt(const std::string& plaintext) const override {
        std::string result = plaintext;
        for (auto& c : result) c ^= 0x42;
        return result;
    }
    std::string decrypt(const std::string& ciphertext) const override {
        std::string result = ciphertext;
        for (auto& c : result) c ^= 0x42;
        return result;
    }
    std::string getWrapAlgorithm() const override { return "RSA/NONE/PKCS1Padding"; }
    std::string getMatDesc() const override { return R"({"desc":"test"})"; }
};

} // namespace

// -- RandomBytes tests (v1: GenerateKeyTest / GenerateIVTest) --

TEST(CipherTest, RandomBytes_Generate32) {
    unsigned char buf1[32], buf2[32];
    EXPECT_TRUE(crypto::RandomBytes(buf1, 32));
    EXPECT_TRUE(crypto::RandomBytes(buf2, 32));
    EXPECT_EQ(32u, sizeof(buf1));
    EXPECT_EQ(32u, sizeof(buf2));
    EXPECT_NE(std::string(reinterpret_cast<char*>(buf1), 32),
              std::string(reinterpret_cast<char*>(buf2), 32));
}

TEST(CipherTest, RandomBytes_Generate16) {
    unsigned char buf1[16], buf2[16];
    EXPECT_TRUE(crypto::RandomBytes(buf1, 16));
    EXPECT_TRUE(crypto::RandomBytes(buf2, 16));
    EXPECT_EQ(16u, sizeof(buf1));
    EXPECT_NE(std::string(reinterpret_cast<char*>(buf1), 16),
              std::string(reinterpret_cast<char*>(buf2), 16));
}

TEST(CipherTest, RandomBytes_NonZero) {
    unsigned char buf[64] = {};
    EXPECT_TRUE(crypto::RandomBytes(buf, sizeof(buf)));
    int nonZero = 0;
    for (auto b : buf) {
        if (b != 0) nonZero++;
    }
    EXPECT_GT(nonZero, 0);
}

TEST(CipherTest, RandomBytes_ZeroLength) {
    unsigned char buf[1] = {0xAA};
    EXPECT_TRUE(crypto::RandomBytes(buf, 0));
    EXPECT_EQ(buf[0], 0xAA);
}

// -- AesCtrCipher tests (v1: AES256_CTRTest) --

TEST(CipherTest, AesCtr_Encrypt16Bytes) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(16, '\0');
    std::memcpy(data.data(), kData16, 16);

    std::vector<uint8_t> encrypted(16);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 16);
    }

    std::string encBase64 = utils::Base64Encode(
        std::string(encrypted.begin(), encrypted.end()));
    EXPECT_EQ("wrnuWSenVochXx3m0QzCBQ==", encBase64);
}

TEST(CipherTest, AesCtr_Decrypt16Bytes) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(16, '\0');
    std::memcpy(data.data(), kData16, 16);

    std::vector<uint8_t> encrypted(16);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 16);
    }

    std::vector<uint8_t> decrypted(16);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(encrypted.data(), decrypted.data(), 16);
    }

    EXPECT_EQ(data, std::string(decrypted.begin(), decrypted.end()));
}

TEST(CipherTest, AesCtr_EncryptDecrypt32Bytes) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(32, '\0');
    std::memcpy(data.data(), kData32, 32);

    std::vector<uint8_t> encrypted(32);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 32);
    }
    EXPECT_NE(data, std::string(encrypted.begin(), encrypted.end()));

    std::vector<uint8_t> decrypted(32);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(encrypted.data(), decrypted.data(), 32);
    }
    EXPECT_EQ(data, std::string(decrypted.begin(), decrypted.end()));
}

// v1: AES256_CTRTest streaming (Encrypt 32 bytes in two 16-byte chunks)
TEST(CipherTest, AesCtr_StreamingProcess) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(32, '\0');
    std::memcpy(data.data(), kData32, 32);

    std::vector<uint8_t> oneShot(32);
    std::vector<uint8_t> streaming(32);

    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       oneShot.data(), 32);
    }
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       streaming.data(), 16);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data() + 16),
                       streaming.data() + 16, 16);
    }

    EXPECT_EQ(oneShot, streaming);
}

// v1: AES256_CTRCounterTest
TEST(CipherTest, AesCtr_CounterIncrement) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data32(32, '\0');
    std::memcpy(data32.data(), kData32, 32);
    std::string data16(16, '\0');
    std::memcpy(data16.data(), kData16, 16);

    std::vector<uint8_t> out32(32);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data32.data()),
                       out32.data(), 32);
    }

    // Manually increment IV counter by 1 (low 8 bytes as big-endian counter)
    std::string iv1 = iv;
    uint64_t counter = 0;
    for (int i = 8; i < 16; i++) {
        counter = (counter << 8) | static_cast<uint8_t>(iv1[static_cast<size_t>(i)]);
    }
    counter += 1;
    for (int i = 15; i >= 8; i--) {
        iv1[static_cast<size_t>(i)] = static_cast<char>(counter & 0xFF);
        counter >>= 8;
    }

    std::vector<uint8_t> out16(16);
    {
        crypto::AesCtrCipher cipher(key, iv1);
        cipher.process(reinterpret_cast<const uint8_t*>(data16.data()),
                       out16.data(), 16);
    }

    // Second block of 32-byte encryption should match separate encryption with iv+1
    EXPECT_EQ(std::vector<uint8_t>(out32.begin() + 16, out32.end()),
              out16);
}

TEST(CipherTest, AesCtr_DifferentKeys_DifferentCiphertext) {
    std::string key1(32, '\0');
    std::memcpy(key1.data(), kKey32, 32);

    std::string key2(32, '\0');
    std::memcpy(key2.data(), "abcdefghijklmnopabcdefghijklmnop", 32);

    std::string iv(16, '\0');
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(32, '\0');
    std::memcpy(data.data(), kData32, 32);

    std::vector<uint8_t> enc1(32), enc2(32);

    {
        crypto::AesCtrCipher c1(key1, iv);
        c1.process(reinterpret_cast<const uint8_t*>(data.data()),
                   enc1.data(), 32);
    }
    {
        crypto::AesCtrCipher c2(key2, iv);
        c2.process(reinterpret_cast<const uint8_t*>(data.data()),
                   enc2.data(), 32);
    }

    EXPECT_NE(enc1, enc2);
}

// -- AesCtrContentCipher tests --

TEST(CipherTest, ContentCipher_EncryptDecryptRoundTrip) {
    crypto::CipherData cd;
    cd.key.resize(32);
    cd.iv.resize(16);
    std::memcpy(cd.key.data(), kKey32, 32);
    std::memcpy(cd.iv.data(), kIV16, 16);

    crypto::AesCtrContentCipher cc(cd);

    std::string plaintext(32, '\0');
    std::memcpy(plaintext.data(), kData32, 32);

    auto body = std::make_shared<StringContent>(plaintext);
    auto encrypted = cc.encryptContent(body);

    auto src = encrypted->spanSource();
    auto encBytes = src->readToEnd();
    std::string encStr(encBytes.begin(), encBytes.end());
    EXPECT_EQ(encStr.size(), plaintext.size());
    EXPECT_NE(encStr, plaintext);

    crypto::AesCtrContentCipher cc2(cd);
    auto userStream = std::make_shared<std::stringstream>();
    auto writer = std::make_shared<OStreamWriter>(userStream);
    auto decWriter = cc2.decryptContent(writer, static_cast<int64_t>(encStr.size()));
    decWriter->write(reinterpret_cast<const uint8_t*>(encStr.data()), encStr.size());

    EXPECT_EQ(plaintext, userStream->str());
}

TEST(CipherTest, ContentCipher_GetEncryptedLen) {
    crypto::CipherData cd;
    cd.key.resize(32);
    cd.iv.resize(16);
    crypto::AesCtrContentCipher cc(cd);
    EXPECT_EQ(100, cc.getEncryptedLen(100));
    EXPECT_EQ(0, cc.getEncryptedLen(0));
    EXPECT_EQ(1, cc.getEncryptedLen(1));
}

TEST(CipherTest, ContentCipher_AlignLen) {
    crypto::CipherData cd;
    cd.key.resize(32);
    cd.iv.resize(16);
    crypto::AesCtrContentCipher cc(cd);
    EXPECT_EQ(16, cc.getAlignLen());
}

TEST(CipherTest, ContentCipher_Clone) {
    crypto::CipherData cd;
    cd.key.resize(32);
    cd.iv.resize(16);
    std::memcpy(cd.key.data(), kKey32, 32);
    std::memcpy(cd.iv.data(), kIV16, 16);

    crypto::AesCtrContentCipher original(cd);
    auto cloned = original.clone();

    std::string plaintext(32, '\0');
    std::memcpy(plaintext.data(), kData32, 32);

    auto body1 = std::make_shared<StringContent>(plaintext);
    auto body2 = std::make_shared<StringContent>(plaintext);

    auto enc1 = original.encryptContent(body1);
    auto enc2 = cloned->encryptContent(body2);

    auto bytes1 = enc1->spanSource()->readToEnd();
    auto bytes2 = enc2->spanSource()->readToEnd();

    EXPECT_EQ(bytes1, bytes2);
}

// -- AesCtrCipherBuilder tests --

TEST(CipherTest, CipherBuilder_Create) {
    auto mc = std::make_shared<MockMasterCipher>();
    auto builder = crypto::CreateAesCtrCipherBuilder(mc);
    ASSERT_NE(nullptr, builder);

    auto cipher = builder->create();
    ASSERT_NE(nullptr, cipher);

    auto& cd = cipher->getCipherData();
    EXPECT_EQ(32u, cd.key.size());
    EXPECT_EQ(16u, cd.iv.size());
    EXPECT_FALSE(cd.encryptedKey.empty());
    EXPECT_FALSE(cd.encryptedIV.empty());

    std::string decKey = mc->decrypt(cd.encryptedKey);
    std::string decIV = mc->decrypt(cd.encryptedIV);
    EXPECT_EQ(cd.key, decKey);
    EXPECT_EQ(cd.iv, decIV);
}

TEST(CipherTest, CipherBuilder_Create_UniqueMaterial) {
    auto mc = std::make_shared<MockMasterCipher>();
    auto builder = crypto::CreateAesCtrCipherBuilder(mc);

    auto c1 = builder->create();
    auto c2 = builder->create();
    ASSERT_NE(nullptr, c1);
    ASSERT_NE(nullptr, c2);

    EXPECT_NE(c1->getCipherData().key, c2->getCipherData().key);
    EXPECT_NE(c1->getCipherData().iv, c2->getCipherData().iv);
}

TEST(CipherTest, CipherBuilder_FromEnvelope) {
    auto mc = std::make_shared<MockMasterCipher>();
    auto builder = crypto::CreateAesCtrCipherBuilder(mc);

    auto original = builder->create();
    ASSERT_NE(nullptr, original);
    auto& origData = original->getCipherData();

    crypto::Envelope env;
    env.cipherKey = utils::Base64Encode(origData.encryptedKey);
    env.iv = utils::Base64Encode(origData.encryptedIV);
    env.cekAlg = "AES/CTR/NoPadding";
    env.wrapAlg = "RSA/NONE/PKCS1Padding";

    auto restored = builder->fromEnvelope(env);
    ASSERT_NE(nullptr, restored);

    EXPECT_EQ(origData.key, restored->getCipherData().key);
    EXPECT_EQ(origData.iv, restored->getCipherData().iv);
}

TEST(CipherTest, CipherBuilder_Metadata) {
    auto mc = std::make_shared<MockMasterCipher>();
    auto builder = crypto::CreateAesCtrCipherBuilder(mc);

    auto& meta = builder->getCipherMetadata();
    EXPECT_EQ("RSA/NONE/PKCS1Padding", meta.wrapAlgorithm);
    EXPECT_EQ("AES/CTR/NoPadding", meta.cekAlgorithm);
    EXPECT_EQ(R"({"desc":"test"})", meta.matDesc);
    EXPECT_EQ(16, builder->getAlignLen());
}

TEST(CipherTest, CipherBuilder_FromEnvelope_InvalidKey) {
    auto mc = std::make_shared<MockMasterCipher>();
    auto builder = crypto::CreateAesCtrCipherBuilder(mc);

    crypto::Envelope env;
    env.cipherKey = "";
    env.iv = "";
    env.cekAlg = "AES/CTR/NoPadding";
    env.wrapAlg = "RSA/NONE/PKCS1Padding";

    auto cipher = builder->fromEnvelope(env);
    EXPECT_EQ(nullptr, cipher);
}

// -- SeekTo tests --

TEST(CipherTest, ContentCipher_SeekTo_AlignedOffset) {
    crypto::CipherData cd;
    cd.key.resize(32);
    cd.iv.resize(16);
    std::memcpy(cd.key.data(), kKey32, 32);
    std::memcpy(cd.iv.data(), kIV16, 16);

    std::string data(128, '\0');
    for (size_t i = 0; i < 128; i++) {
        data[i] = static_cast<char>((i % 26) + 'a');
    }

    std::vector<uint8_t> fullEnc(128);
    {
        crypto::AesCtrCipher cipher(cd.key, cd.iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       fullEnc.data(), 128);
    }

    crypto::AesCtrContentCipher cc(cd);
    cc.seekTo(64);

    auto partBody = std::make_shared<StringContent>(data.substr(64));
    auto encPart = cc.encryptContent(partBody);
    auto partBytes = encPart->spanSource()->readToEnd();

    EXPECT_EQ(64u, partBytes.size());
    EXPECT_EQ(std::vector<uint8_t>(fullEnc.begin() + 64, fullEnc.end()),
              partBytes);
}

TEST(CipherTest, AesCtr_KnownVector_32Bytes) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    std::string data(32, '\0');
    std::memcpy(data.data(), kData32, 32);

    std::vector<uint8_t> encrypted(32);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 32);
    }

    std::string encBase64 = utils::Base64Encode(
        std::string(encrypted.begin(), encrypted.end()));
    EXPECT_EQ("wrnuWSenVochXx3m0QzCBasLNayhLlbwFpYw7TFbVEQ=", encBase64);
}

TEST(CipherTest, AesCtr_KnownVector_33Bytes) {
    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKey32, 32);
    std::memcpy(iv.data(), kIV16, 16);

    const char* kData33 = "123456789012345612345678901234561";
    std::string data(33, '\0');
    std::memcpy(data.data(), kData33, 33);

    std::vector<uint8_t> encrypted(33);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 33);
    }

    std::string encBase64 = utils::Base64Encode(
        std::string(encrypted.begin(), encrypted.end()));
    EXPECT_EQ("wrnuWSenVochXx3m0QzCBasLNayhLlbwFpYw7TFbVETO", encBase64);
}

TEST(CipherTest, AesCtr_KnownVector_DifferentKeyIV) {
    const char* kKeyAbc = "abcdefghijklmnopabcdefghijklmnop";
    const char* kIvABC = "ABCDEFGHIJKLMNOP";
    const char* kData34 = "1234567890123456123456789012345612";

    std::string key(32, '\0');
    std::string iv(16, '\0');
    std::memcpy(key.data(), kKeyAbc, 32);
    std::memcpy(iv.data(), kIvABC, 16);

    std::string data(34, '\0');
    std::memcpy(data.data(), kData34, 34);

    std::vector<uint8_t> encrypted(34);
    {
        crypto::AesCtrCipher cipher(key, iv);
        cipher.process(reinterpret_cast<const uint8_t*>(data.data()),
                       encrypted.data(), 34);
    }

    std::string encBase64 = utils::Base64Encode(
        std::string(encrypted.begin(), encrypted.end()));
    EXPECT_EQ("io0Tu+Y/w0lOuICPqXL7o95ra2eymWZcE+l2vFjySoYZ+g==", encBase64);
}

} // namespace alibabacloud::oss2
