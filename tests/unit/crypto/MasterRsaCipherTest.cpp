#include <gtest/gtest.h>

#include "alibabacloud/oss2/crypto/MasterRsaCipher.h"

namespace alibabacloud::oss2 {

namespace {

const char* kPublicKey = R"(-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6VIgfsPq979hYMNEoDG1pfG58
1FXN7d2GwPPR9d5a8O7+kVGy/PhxpbOWBqKg+JTmxv7AkMmlndAf18zoY5UnNW+d
58mYZrPODBiepxdjUD/tYI2NQcgzCs3slRRRb5faa5a+l6biUJWNBf1uKW4y7JPD
8eEIseQKCW0oRJVLtQIDAQAB
-----END PUBLIC KEY-----)";

const char* kPrivateKey = R"(-----BEGIN PRIVATE KEY-----
MIICeAIBADANBgkqhkiG9w0BAQEFAASCAmIwggJeAgEAAoGBALpUiB+w+r3v2Fgw
0SgMbWl8bnzUVc3t3YbA89H13lrw7v6RUbL8+HGls5YGoqD4lObG/sCQyaWd0B/X
zOhjlSc1b53nyZhms84MGJ6nF2NQP+1gjY1ByDMKzeyVFFFvl9prlr6XpuJQlY0F
/W4pbjLsk8Px4Qix5AoJbShElUu1AgMBAAECgYEAmvNZECG5IuKl4xEVnlxXUHWt
3BkoEcxRgJJNMLlqY+4gkYp/in1cjgXiRkzWSU7vZMrvZ2wAhL2sKg7n1AmcKkbg
K8tiUnfvADZRuFDWsEfl2kEP+2/cJTJ920j0ItSxlHcMYFZdjAYbQoYxg8TF7ker
c9Qf35Ca476Veok7ggECQQDqIt57qgHO7q/3TOuvhgD002v4m+2mWkhI6Iv9fOxT
paZ1ePOwUtKxkJ5okqx93OKNgRx5djvNQ1rT+pCHYEqhAkEAy7rWtDnAq2qXahHh
49NKbTT+Iegh90WcE1T5vy649eV/BTzQ7AkZuzTEEd7523i5PndAtyLgqCc2QdLm
W7hclQJBAKKwK+u9y5fgHoE1/6Zs9IkpxyJuJomqvgN7Ipq2jPfqaGnD64AfbKtZ
E9kR4a1rKDiu9/wl/ZO5M4mL15VZgUECQQC4g/PJLzVNCzEvpBqOmOMjnYc9dlys
86Kz75ZyjQJ/0ucD+1zNKkDfyJ58ARMSr3g3FxLJyxDluv3tB/ISyBsxAkA1Nn8f
IyjCYM8gL6f0TF0tKt/gnJ0BFJ+e0Vs+zLXigLpyIqlF20C0C2JTyz14BRJaLvGh
o8nOMAdZuYZIcdyS
-----END PRIVATE KEY-----)";

const char* kPublicKey2 = R"(-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCokfiAVXXf5ImFzKDw+XO/UByW
6mse2QsIgz3ZwBtMNu59fR5zttSx+8fB7vR4CN3bTztrP9A6bjoN0FFnhlQ3vNJC
5MFO1PByrE/MNd5AAfSVba93I6sx8NSk5MzUCA4NJzAUqYOEWGtGBcom6kEF6MmR
1EKib1Id8hpooY5xaQIDAQAB
-----END PUBLIC KEY-----)";

const char* kPrivateKey2 = R"(-----BEGIN RSA PRIVATE KEY-----
MIICWwIBAAKBgQCokfiAVXXf5ImFzKDw+XO/UByW6mse2QsIgz3ZwBtMNu59fR5z
ttSx+8fB7vR4CN3bTztrP9A6bjoN0FFnhlQ3vNJC5MFO1PByrE/MNd5AAfSVba93
I6sx8NSk5MzUCA4NJzAUqYOEWGtGBcom6kEF6MmR1EKib1Id8hpooY5xaQIDAQAB
AoGAOPUZgkNeEMinrw31U3b2JS5sepG6oDG2CKpPu8OtdZMaAkzEfVTJiVoJpP2Y
nPZiADhFW3e0ZAnak9BPsSsySRaSNmR465cG9tbqpXFKh9Rp/sCPo4Jq2n65yood
JBrnGr6/xhYvNa14sQ6xjjfSgRNBSXD1XXNF4kALwgZyCAECQQDV7t4bTx9FbEs5
36nAxPsPM6aACXaOkv6d9LXI7A0J8Zf42FeBV6RK0q7QG5iNNd1WJHSXIITUizVF
6aX5NnvFAkEAybeXNOwUvYtkgxF4s28s6gn11c5HZw4/a8vZm2tXXK/QfTQrJVXp
VwxmSr0FAajWAlcYN/fGkX1pWA041CKFVQJAG08ozzekeEpAuByTIOaEXgZr5MBQ
gBbHpgZNBl8Lsw9CJSQI15wGfv6yDiLXsH8FyC9TKs+d5Tv4Cvquk0efOQJAd9OC
lCKFs48hdyaiz9yEDsc57PdrvRFepVdj/gpGzD14mVerJbOiOF6aSV19ot27u4on
Td/3aifYs0CveHzFPQJAWb4LCDwqLctfzziG7/S7Z74gyq5qZF4FUElOAZkz718E
yZvADwuz/4aK0od0lX9c4Jp7Mo5vQ4TvdoBnPuGoyw==
-----END RSA PRIVATE KEY-----)";

// 2048-bit key for testing larger key sizes
const char* kPublicKey2048 = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvJy2aTjksDzS2QdJsfJF
/yXHwIxf5rKSUEqGC8qZVOFfbfKhK1lwlpX1NH5Izlxqb2n/ezq9bY9l8ro+Tq6b
T5wS8Ep1h1uzW0TWlx5uCBlpqTgz152Knevf+Rhd0YYE4fQzM40yooqgGWPzuu+6
Wb0q5z82CaVvuJHh6CUFQJtwnAa+xdsuiCAgqUwfJqDKdyLTMASbZ46sYk0FDOKN
hc5pFxiXR0NqQqstTsjtXncbqB9KSSStJ9ghivvQuXwygcMoFRTXMJwYmBY1KikU
KMOUCL+TE6vktUF2IFCIwf3YQFsNI+IrtYNlHE6iEg0aKIB7JG1vFlBfajpTiQgl
GQIDAQAB
-----END PUBLIC KEY-----)";

const char* kPrivateKey2048 = R"(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC8nLZpOOSwPNLZ
B0mx8kX/JcfAjF/mspJQSoYLyplU4V9t8qErWXCWlfU0fkjOXGpvaf97Or1tj2Xy
uj5OrptPnBLwSnWHW7NbRNaXHm4IGWmpODPXnYqd69/5GF3RhgTh9DMzjTKiiqAZ
Y/O677pZvSrnPzYJpW+4keHoJQVAm3CcBr7F2y6IICCpTB8moMp3ItMwBJtnjqxi
TQUM4o2FzmkXGJdHQ2pCqy1OyO1edxuoH0pJJK0n2CGK+9C5fDKBwygVFNcwnBiY
FjUqKRQow5QIv5MTq+S1QXYgUIjB/dhAWw0j4iu1g2UcTqISDRoogHskbW8WUF9q
OlOJCCUZAgMBAAECggEAGZY+PhK0c3QiO8RRVbftkaopFGX2se/ntBy9XTwkMO+i
Eo2nxPRijGCA3e4ufPrsitDGZ+FAHBd9V8BhqNWRxusAEWOzLfmytd77fY9okztf
VNAwYupsXkrGx57BRiX4OO737eEUeBh2P6Y076yLNewDtftNSWG4FkHeyQS8rFbY
zVBuvHjrzfEKyUfmqd9tK155Hhd7eusm1YJ75VYx6emlD9EE22Sk4bXCCI5Bh7Zl
548chJ4V1HQ6PCCj9BpBcMnGifzjG8EU1bYwbT669daZwB2WHgsBEAX/BfrM61Dq
BX/Xq7Ij5v0FxleJ5r5aWXDWuIpyOrH1vyBt2152QQKBgQDupmG9IhGKkO7V8m/b
oHS5tW/vZ2+Jw4/rnM2JWpoXwO7xg1MmC0D8zKcLNBXeobJhvBsEVQvdCMokmSpn
NOaNJ9kSqnSl+vxDycmHSlh8b656xjVlexLEx5A3DysJvhaYdI4/Uc8TfB/jWk+L
sNOrhGU9fMamamvczierIWndkwKBgQDKUw5Qqd0PHOhm+x8RaNGQXncFqu0Ye2pa
yE0Sbgm/ofysvzW4cLehpSmmYEIMG8DNVJtEnhmWTl8cQzQnp1fAXlohv0wer5de
NVWQIvLo1F9wI+v50MWvgWBXwQHwLFq7sgYDZCTERBymq85mIbdT2WFDhw+rMRH0
ODqE5c3+IwKBgGW8I/pepZ+ufUJTYX/8/QWV5SvnqlLOPXIxnCUrrHjn1HS8iRu0
vHWIQMWz5IbN459qcxH7t1z4vEOxz7PDh20xSYZ9h9CiGBxFz1WPSf1yFq1cBbNH
Lg8ZC8+M9cnncPZ46ZLwqxghV+6xtytTrEh33jjCEmUrBORSNfLsAZdlAoGAFvcs
hc1yMTf3zVCt6xz5xKhkXDlVplTD8sAPt4rUAnORqc4ee+wXe/qyapc8iAFSdjwn
T7eeceg9dYjPT7z4AfbzxibfrhACX4gwSSceaX1JxAHf1EB1YAGQfQWEgc2XEv0X
H6VrYvfURLr1t7QWCid/mdmn1qfAQPds9Q7cvf8CgYEAoLQHZsQdbhGBxtpdFc2r
98xs6ltwEpIYUofKrb+efyhpM8PMvsJnehLMdwbh2ZV2qdBGbpxYmBcSSev4+Zvx
r1pRhLNEyTW9IOKpZOMUbBIyTpgVEOL5SHDHYPnIcNaogGAYR79oXekTrLhZpHvG
VBIJ/MGrJ+PpwOjY0Y/v8jE=
-----END PRIVATE KEY-----)";

} // namespace

TEST(MasterRsaCipherTest, EncryptDecrypt) {
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey);

    std::string plaintext = "test key material 32 bytes long!";
    auto encResult = cipher.encrypt(plaintext);
    ASSERT_TRUE(std::holds_alternative<std::string>(encResult));
    auto& encrypted = std::get<std::string>(encResult);
    EXPECT_NE(plaintext, encrypted);

    auto decResult = cipher.decrypt(encrypted);
    ASSERT_TRUE(std::holds_alternative<std::string>(decResult));
    EXPECT_EQ(plaintext, std::get<std::string>(decResult));
}

TEST(MasterRsaCipherTest, EncryptDecrypt_2048bit) {
    crypto::MasterRsaCipher cipher(kPublicKey2048, kPrivateKey2048);

    std::string plaintext = "test key material 32 bytes long!";
    auto encResult = cipher.encrypt(plaintext);
    ASSERT_TRUE(std::holds_alternative<std::string>(encResult));
    auto& encrypted = std::get<std::string>(encResult);
    EXPECT_GT(encrypted.size(), plaintext.size());

    auto decResult = cipher.decrypt(encrypted);
    ASSERT_TRUE(std::holds_alternative<std::string>(decResult));
    EXPECT_EQ(plaintext, std::get<std::string>(decResult));
}

TEST(MasterRsaCipherTest, WrapAlgorithm) {
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey);
    EXPECT_EQ("RSA/NONE/PKCS1Padding", cipher.getWrapAlgorithm());
}

TEST(MasterRsaCipherTest, MatDesc_WithDescription) {
    std::map<std::string, std::string> desc{{"provider", "test"}, {"version", "1"}};
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey, desc);
    EXPECT_EQ(R"({"provider":"test","version":"1"})", cipher.getMatDesc());
}

TEST(MasterRsaCipherTest, MatDesc_Empty) {
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey);
    EXPECT_EQ("{}", cipher.getMatDesc());
}

TEST(MasterRsaCipherTest, DifferentKeys_CannotCrossDecrypt) {
    crypto::MasterRsaCipher cipher1(kPublicKey, kPrivateKey);
    crypto::MasterRsaCipher cipher2(kPublicKey2, kPrivateKey2);

    std::string plaintext = "cross-key decryption test data!!";

    auto enc1 = cipher1.encrypt(plaintext);
    auto enc2 = cipher2.encrypt(plaintext);
    ASSERT_TRUE(std::holds_alternative<std::string>(enc1));
    ASSERT_TRUE(std::holds_alternative<std::string>(enc2));

    auto dec1 = cipher1.decrypt(std::get<std::string>(enc1));
    ASSERT_TRUE(std::holds_alternative<std::string>(dec1));
    EXPECT_EQ(plaintext, std::get<std::string>(dec1));

    auto dec2 = cipher2.decrypt(std::get<std::string>(enc2));
    ASSERT_TRUE(std::holds_alternative<std::string>(dec2));
    EXPECT_EQ(plaintext, std::get<std::string>(dec2));

    auto cross1 = cipher2.decrypt(std::get<std::string>(enc1));
    auto cross2 = cipher1.decrypt(std::get<std::string>(enc2));
    EXPECT_TRUE(std::holds_alternative<std::error_code>(cross1) ||
                std::get<std::string>(cross1) != plaintext);
    EXPECT_TRUE(std::holds_alternative<std::error_code>(cross2) ||
                std::get<std::string>(cross2) != plaintext);
}

TEST(MasterRsaCipherTest, InvalidKey_EncryptReturnsError) {
    crypto::MasterRsaCipher cipher("invalid-pem", "invalid-pem");
    auto result = cipher.encrypt("test data");
    ASSERT_TRUE(std::holds_alternative<std::error_code>(result));
    auto& ec = std::get<std::error_code>(result);
    EXPECT_FALSE(ec.message().empty());
}

TEST(MasterRsaCipherTest, InvalidKey_DecryptReturnsError) {
    crypto::MasterRsaCipher cipher("invalid-pem", "invalid-pem");
    auto result = cipher.decrypt("some ciphertext");
    ASSERT_TRUE(std::holds_alternative<std::error_code>(result));
    auto& ec = std::get<std::error_code>(result);
    EXPECT_FALSE(ec.message().empty());
}

TEST(MasterRsaCipherTest, EncryptTwice_ProducesDifferentCiphertext) {
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey);
    std::string plaintext = "determinism test 32 bytes long!!";

    auto r1 = cipher.encrypt(plaintext);
    auto r2 = cipher.encrypt(plaintext);
    ASSERT_TRUE(std::holds_alternative<std::string>(r1));
    ASSERT_TRUE(std::holds_alternative<std::string>(r2));
    // PKCS1 v1.5 padding is randomized
    EXPECT_NE(std::get<std::string>(r1), std::get<std::string>(r2));

    auto d1 = cipher.decrypt(std::get<std::string>(r1));
    auto d2 = cipher.decrypt(std::get<std::string>(r2));
    ASSERT_TRUE(std::holds_alternative<std::string>(d1));
    ASSERT_TRUE(std::holds_alternative<std::string>(d2));
    EXPECT_EQ(plaintext, std::get<std::string>(d1));
    EXPECT_EQ(plaintext, std::get<std::string>(d2));
}

TEST(MasterRsaCipherTest, MatDesc_SpecialChars) {
    std::map<std::string, std::string> desc{{"key\"with\\quotes", "val\"ue"}};
    crypto::MasterRsaCipher cipher(kPublicKey, kPrivateKey, desc);
    std::string expected = R"---({"key\"with\\quotes":"val\"ue"})---";
    EXPECT_EQ(expected, cipher.getMatDesc());
}

} // namespace alibabacloud::oss2
