#include <gtest/gtest.h>
#include <fstream>
#include <random>
#include <sstream>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/MasterRsaCipher.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

namespace alibabacloud {
namespace oss2 {

static const char* kPublicKey =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6VIgfsPq979hYMNEoDG1pfG58\n"
    "1FXN7d2GwPPR9d5a8O7+kVGy/PhxpbOWBqKg+JTmxv7AkMmlndAf18zoY5UnNW+d\n"
    "58mYZrPODBiepxdjUD/tYI2NQcgzCs3slRRRb5faa5a+l6biUJWNBf1uKW4y7JPD\n"
    "8eEIseQKCW0oRJVLtQIDAQAB\n"
    "-----END PUBLIC KEY-----";

static const char* kPrivateKey =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXgIBAAKBgQC6VIgfsPq979hYMNEoDG1pfG581FXN7d2GwPPR9d5a8O7+kVGy\n"
    "/PhxpbOWBqKg+JTmxv7AkMmlndAf18zoY5UnNW+d58mYZrPODBiepxdjUD/tYI2N\n"
    "QcgzCs3slRRRb5faa5a+l6biUJWNBf1uKW4y7JPD8eEIseQKCW0oRJVLtQIDAQAB\n"
    "AoGBAJrzWRAhuSLipeMRFZ5cV1B1rdwZKBHMUYCSTTC5amPuIJGKf4p9XI4F4kZM\n"
    "1klO72TK72dsAIS9rCoO59QJnCpG4CvLYlJ37wA2UbhQ1rBH5dpBD/tv3CUyfdtI\n"
    "9CLUsZR3DGBWXYwGG0KGMYPExe5Hq3PUH9+QmuO+lXqJO4IBAkEA6iLee6oBzu6v\n"
    "90zrr4YA9NNr+JvtplpISOiL/XzsU6WmdXjzsFLSsZCeaJKsfdzijYEceXY7zUNa\n"
    "0/qQh2BKoQJBAMu61rQ5wKtql2oR4ePTSm00/iHoIfdFnBNU+b8uuPXlfwU80OwJ\n"
    "Gbs0xBHe+dt4uT53QLci4KgnNkHS5lu4XJUCQQCisCvrvcuX4B6BNf+mbPSJKcci\n"
    "biaJqr4DeyKatoz36mhpw+uAH2yrWRPZEeGtayg4rvf8Jf2TuTOJi9eVWYFBAkEA\n"
    "uIPzyS81TQsxL6QajpjjI52HPXZcrPOis++Wco0Cf9LnA/tczSpA38iefAETEq94\n"
    "NxcSycsQ5br97QfyEsgbMQJANTZ/HyMowmDPIC+n9ExdLSrf4JydARSfntFbPsy1\n"
    "4oC6ciKpRdtAtAtiU8s9eAUSWi7xoaPJzjAHWbmGSHHckg==\n"
    "-----END RSA PRIVATE KEY-----";

static std::string genRandomData(size_t length) {
    static std::mt19937 rng(std::random_device{}());
    std::string data(length, '\0');
    for (size_t i = 0; i < length; i++) {
        data[i] = static_cast<char>(rng() & 0xFF);
    }
    return data;
}

static std::string readStream(std::iostream& stream) {
    std::ostringstream oss;
    oss << stream.rdbuf();
    return oss.str();
}

class EncryptionObjectTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        auto plainClient = sync::ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = plainClient->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        ASSERT_TRUE(outcome.has_value());

        auto provider = std::make_shared<StaticCredentialsProvider>(
                Config::AccessKeyId, Config::AccessKeySecret);
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;

        auto masterCipher = std::make_shared<crypto::MasterRsaCipher>(kPublicKey, kPrivateKey);
        crypto::EncryptionConfiguration encConfig;
        encConfig.masterCipher = masterCipher;
        encClient_ = std::make_shared<OSSEncryptionClient>(config, std::move(encConfig));
        plainClient_ = plainClient;
    }

    static void TearDownTestCase() {
        sync::ClientHelper::CleanBucketsByPrefix(bucketName_);
        encClient_.reset();
        plainClient_.reset();
    }

public:
    static std::shared_ptr<OSSEncryptionClient> encClient_;
    static std::shared_ptr<OSSClient> plainClient_;
    static std::string bucketName_;
};

std::shared_ptr<OSSEncryptionClient> EncryptionObjectTest::encClient_ = nullptr;
std::shared_ptr<OSSClient> EncryptionObjectTest::plainClient_ = nullptr;
std::string EncryptionObjectTest::bucketName_ = "";

// --- Basic PutObject + GetObject ---

TEST_F(EncryptionObjectTest, PutGet_SmallObject) {
    std::string key = "enc-test-small";
    std::string content = "hello, client-side encryption!";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content, got);
}

TEST_F(EncryptionObjectTest, PutGet_LargeObject) {
    std::string key = "enc-test-large";
    std::string content = genRandomData(1024 * 1024 + 100);

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.size(), got.size());
    EXPECT_EQ(content, got);
}

TEST_F(EncryptionObjectTest, PutGet_VariousSizes) {
    std::vector<size_t> sizes = {1, 15, 16, 17, 100, 1000, 16 * 1024 + 7};
    for (auto sz : sizes) {
        std::string key = "enc-test-size-" + std::to_string(sz);
        std::string content = genRandomData(sz);

        auto putOutcome = encClient_->putObject(
                models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString(content)));
        ASSERT_TRUE(putOutcome.has_value()) << "put failed for size=" << sz;

        auto getOutcome = encClient_->getObject(
                models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key));
        ASSERT_TRUE(getOutcome.has_value()) << "get failed for size=" << sz;

        auto got = readStream(*getOutcome.value().getBody());
        EXPECT_EQ(content, got) << "content mismatch for size=" << sz;
    }
}

// --- Range Get ---

TEST_F(EncryptionObjectTest, GetObject_RangeFromStart) {
    std::string key = "enc-test-range";
    std::string content = "0123456789abcdefghijklmnopqrstuvwxyz";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setRange("bytes=0-9"));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.substr(0, 10), got);
}

TEST_F(EncryptionObjectTest, GetObject_RangeCrossBlock) {
    std::string key = "enc-test-range-cross";
    std::string content = "0123456789abcdefghijklmnopqrstuvwxyz";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setRange("bytes=0-19"));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.substr(0, 20), got);
}

TEST_F(EncryptionObjectTest, GetObject_RangeMiddle) {
    std::string key = "enc-test-range-middle";
    std::string content = "0123456789abcdefghijklmnopqrstuvwxyz";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setRange("bytes=10-19"));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.substr(10, 10), got);
}

TEST_F(EncryptionObjectTest, GetObject_RangeUnaligned) {
    std::string key = "enc-test-range-unaligned";
    std::string content = "0123456789abcdefghijklmnopqrstuvwxyz";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setRange("bytes=5-"));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.substr(5), got);
}

TEST_F(EncryptionObjectTest, GetObject_RangeAligned) {
    std::string key = "enc-test-range-aligned";
    std::string content = "0123456789abcdefGHIJKLMNOPQRSTUVWXYZ";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setRange("bytes=16-31"));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.substr(16, 16), got);
}

// --- Multipart Upload ---

TEST_F(EncryptionObjectTest, MultipartUpload_Normal) {
    std::string key = "enc-test-multipart";
    int64_t partSize = 102400;
    std::string content = genRandomData(partSize * 3 + 500);
    int64_t dataSize = static_cast<int64_t>(content.size());

    auto initOutcome = encClient_->initiateMultipartUpload(
            models::InitiateMultipartUploadRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setCsePartSize(partSize)
                .setCseDataSize(dataSize));
    ASSERT_TRUE(initOutcome.has_value());

    auto uploadId = initOutcome.value().getUploadId();
    auto ctx = initOutcome.value().getCseMultiPartContext();
    ASSERT_TRUE(ctx != nullptr);

    int partCount = static_cast<int>((dataSize + partSize - 1) / partSize);
    std::vector<models::Part> parts;

    for (int i = 0; i < partCount; i++) {
        int64_t offset = partSize * i;
        int64_t thisPartSize = std::min(partSize, dataSize - offset);
        std::string partData = content.substr(static_cast<size_t>(offset),
                                              static_cast<size_t>(thisPartSize));

        auto partOutcome = encClient_->uploadPart(
                models::UploadPartRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setUploadId(uploadId)
                    .setPartNumber(i + 1)
                    .setBody(RequestBody::FromString(partData))
                    .setCseMultiPartContext(ctx));
        ASSERT_TRUE(partOutcome.has_value()) << "uploadPart failed for part " << (i + 1);

        models::Part part;
        part.partNumber = i + 1;
        part.eTag = partOutcome.value().getETag();
        parts.push_back(part);
    }

    models::CompleteMultipartUpload cmu;
    cmu.setParts(parts);
    auto completeOutcome = encClient_->completeMultipartUpload(
            models::CompleteMultipartUploadRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setUploadId(uploadId)
                .setCompleteMultipartUpload(cmu));
    ASSERT_TRUE(completeOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content.size(), got.size());
    EXPECT_EQ(content, got);
}

TEST_F(EncryptionObjectTest, MultipartUpload_InvalidPartSize) {
    std::string key = "enc-test-multipart-invalid";

    auto initOutcome = encClient_->initiateMultipartUpload(
            models::InitiateMultipartUploadRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setCsePartSize(102401)
                .setCseDataSize(500000));
    EXPECT_FALSE(initOutcome.has_value());
    EXPECT_EQ("ArgumentInvalid", initOutcome.error().getCode());
}

TEST_F(EncryptionObjectTest, MultipartUpload_MissingDataSize) {
    std::string key = "enc-test-multipart-nodata";

    auto initOutcome = encClient_->initiateMultipartUpload(
            models::InitiateMultipartUploadRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setCsePartSize(102400));
    EXPECT_FALSE(initOutcome.has_value());
    EXPECT_EQ("ArgumentInvalid", initOutcome.error().getCode());
}

// --- Unencrypted Passthrough ---

TEST_F(EncryptionObjectTest, GetObject_UnencryptedPassthrough) {
    std::string key = "enc-test-unencrypted";
    std::string content = "plain text content";

    auto putOutcome = plainClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    ASSERT_TRUE(getOutcome.has_value());

    auto got = readStream(*getOutcome.value().getBody());
    EXPECT_EQ(content, got);
}

// --- Error Cases ---

TEST_F(EncryptionObjectTest, PutObject_InvalidRsaKey) {
    auto provider = std::make_shared<StaticCredentialsProvider>(
            Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;

    auto badMaster = std::make_shared<crypto::MasterRsaCipher>("invalid", "invalid");
    crypto::EncryptionConfiguration badEncConfig;
    badEncConfig.masterCipher = badMaster;
    OSSEncryptionClient badClient(config, std::move(badEncConfig));

    auto putOutcome = badClient.putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey("enc-test-bad-key")
                .setBody(RequestBody::FromString("test")));
    EXPECT_FALSE(putOutcome.has_value());
    EXPECT_EQ("EncryptionFailure", putOutcome.error().getCode());
}

TEST_F(EncryptionObjectTest, GetObject_DifferentKey) {
    std::string key = "enc-test-diff-key";
    std::string content = "secret data";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    static const char* kOtherPrivateKey =
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIICWwIBAAKBgQCokfiAVXXf5ImFzKDw+XO/UByW6mse2QsIgz3ZwBtMNu59fR5z\n"
        "ttSx+8fB7vR4CN3bTztrP9A6bjoN0FFnhlQ3vNJC5MFO1PByrE/MNd5AAfSVba93\n"
        "I6sx8NSk5MzUCA4NJzAUqYOEWGtGBcom6kEF6MmR1EKib1Id8hpooY5xaQIDAQAB\n"
        "AoGAOPUZgkNeEMinrw31U3b2JS5sepG6oDG2CKpPu8OtdZMaAkzEfVTJiVoJpP2Y\n"
        "nPZiADhFW3e0ZAnak9BPsSsySRaSNmR465cG9tbqpXFKh9Rp/sCPo4Jq2n65yood\n"
        "JBrnGr6/xhYvNa14sQ6xjjfSgRNBSXD1XXNF4kALwgZyCAECQQDV7t4bTx9FbEs5\n"
        "36nAxPsPM6aACXaOkv6d9LXI7A0J8Zf42FeBV6RK0q7QG5iNNd1WJHSXIITUizVF\n"
        "6aX5NnvFAkEAybeXNOwUvYtkgxF4s28s6gn11c5HZw4/a8vZm2tXXK/QfTQrJVXp\n"
        "VwxmSr0FAajWAlcYN/fGkX1pWA041CKFVQJAG08ozzekeEpAuByTIOaEXgZr5MBQ\n"
        "gBbHpgZNBl8Lsw9CJSQI15wGfv6yDiLXsH8FyC9TKs+d5Tv4Cvquk0efOQJAd9OC\n"
        "lCKFs48hdyaiz9yEDsc57PdrvRFepVdj/gpGzD14mVerJbOiOF6aSV19ot27u4on\n"
        "Td/3aifYs0CveHzFPQJAWb4LCDwqLctfzziG7/S7Z74gyq5qZF4FUElOAZkz718E\n"
        "yZvADwuz/4aK0od0lX9c4Jp7Mo5vQ4TvdoBnPuGoyw==\n"
        "-----END RSA PRIVATE KEY-----";

    static const char* kOtherPublicKey =
        "-----BEGIN PUBLIC KEY-----\n"
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCokfiAVXXf5ImFzKDw+XO/UByW\n"
        "6mse2QsIgz3ZwBtMNu59fR5zttSx+8fB7vR4CN3bTztrP9A6bjoN0FFnhlQ3vNJC\n"
        "5MFO1PByrE/MNd5AAfSVba93I6sx8NSk5MzUCA4NJzAUqYOEWGtGBcom6kEF6MmR\n"
        "1EKib1Id8hpooY5xaQIDAQAB\n"
        "-----END PUBLIC KEY-----";

    auto provider = std::make_shared<StaticCredentialsProvider>(
            Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;

    auto otherMaster = std::make_shared<crypto::MasterRsaCipher>(kOtherPublicKey, kOtherPrivateKey);
    crypto::EncryptionConfiguration otherEncConfig;
    otherEncConfig.masterCipher = otherMaster;
    OSSEncryptionClient otherClient(config, std::move(otherEncConfig));

    auto getOutcome = otherClient.getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    if (getOutcome.has_value()) {
        auto got = readStream(*getOutcome.value().getBody());
        EXPECT_NE(content, got);
    }
}

// --- SinkFactory ---

TEST_F(EncryptionObjectTest, GetObject_WithSinkFactory) {
    std::string key = "enc-test-sinkfactory";
    std::string content = "hello sink factory decryption!";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };

    auto getOutcome = encClient_->getObject(
            models::GetObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setSinkFactory(factory));
    ASSERT_TRUE(getOutcome.has_value());

    EXPECT_EQ(content, userStream->str());
}

// --- Metadata ---

TEST_F(EncryptionObjectTest, PutGet_UserMetadataPreserved) {
    std::string key = "enc-test-metadata";
    std::string content = "metadata test";

    HeaderCollection meta;
    meta["my-key1"] = "value1";
    meta["my-key2"] = "value2";

    auto putOutcome = encClient_->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(key)
                .setBody(RequestBody::FromString(content))
                .setMetadata(meta));
    ASSERT_TRUE(putOutcome.has_value());

    auto headOutcome = plainClient_->headObject(
            models::HeadObjectRequest()
                .setBucket(bucketName_)
                .setKey(key));
    ASSERT_TRUE(headOutcome.has_value());

    auto& headers = headOutcome.value().getHeaders();
    auto it1 = headers.find("x-oss-meta-my-key1");
    auto it2 = headers.find("x-oss-meta-my-key2");
    EXPECT_NE(it1, headers.end());
    EXPECT_NE(it2, headers.end());
    if (it1 != headers.end()) EXPECT_EQ("value1", it1->second);
    if (it2 != headers.end()) EXPECT_EQ("value2", it2->second);

    auto cseKey = headers.find("x-oss-meta-client-side-encryption-key");
    EXPECT_NE(cseKey, headers.end());
}

// --- Cross SDK Compatibility ---

TEST_F(EncryptionObjectTest, CrossSdk_DecryptPreEncryptedData) {
    static const char* kCompatPrivateKey =
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIICWwIBAAKBgQCokfiAVXXf5ImFzKDw+XO/UByW6mse2QsIgz3ZwBtMNu59fR5z\n"
        "ttSx+8fB7vR4CN3bTztrP9A6bjoN0FFnhlQ3vNJC5MFO1PByrE/MNd5AAfSVba93\n"
        "I6sx8NSk5MzUCA4NJzAUqYOEWGtGBcom6kEF6MmR1EKib1Id8hpooY5xaQIDAQAB\n"
        "AoGAOPUZgkNeEMinrw31U3b2JS5sepG6oDG2CKpPu8OtdZMaAkzEfVTJiVoJpP2Y\n"
        "nPZiADhFW3e0ZAnak9BPsSsySRaSNmR465cG9tbqpXFKh9Rp/sCPo4Jq2n65yood\n"
        "JBrnGr6/xhYvNa14sQ6xjjfSgRNBSXD1XXNF4kALwgZyCAECQQDV7t4bTx9FbEs5\n"
        "36nAxPsPM6aACXaOkv6d9LXI7A0J8Zf42FeBV6RK0q7QG5iNNd1WJHSXIITUizVF\n"
        "6aX5NnvFAkEAybeXNOwUvYtkgxF4s28s6gn11c5HZw4/a8vZm2tXXK/QfTQrJVXp\n"
        "VwxmSr0FAajWAlcYN/fGkX1pWA041CKFVQJAG08ozzekeEpAuByTIOaEXgZr5MBQ\n"
        "gBbHpgZNBl8Lsw9CJSQI15wGfv6yDiLXsH8FyC9TKs+d5Tv4Cvquk0efOQJAd9OC\n"
        "lCKFs48hdyaiz9yEDsc57PdrvRFepVdj/gpGzD14mVerJbOiOF6aSV19ot27u4on\n"
        "Td/3aifYs0CveHzFPQJAWb4LCDwqLctfzziG7/S7Z74gyq5qZF4FUElOAZkz718E\n"
        "yZvADwuz/4aK0od0lX9c4Jp7Mo5vQ4TvdoBnPuGoyw==\n"
        "-----END RSA PRIVATE KEY-----";

    struct CompatTestCase {
        std::string encFile;
        std::map<std::string, std::string> cseHeaders;
    };

    std::vector<CompatTestCase> cases = {
        {"cpp-enc-example.jpg", {
            {"client-side-encryption-key", "nyXOp7delQ/MQLjKQMhHLaT0w7u2yQoDLkSnK8MFg/MwYdh4na4/LS8LLbLcM18m8I/ObWUHU775I50sJCpdv+f4e0jLeVRRiDFWe+uo7Puc9j4xHj8YB3QlcIOFQiTxHIB6q+C+RA6lGwqqYVa+n3aV5uWhygyv1MWmESurppg="},
            {"client-side-encryption-start", "De/S3T8wFjx7QPxAAFl7h7TeI2EsZlfCwox4WhLGng5DK2vNXxULmulMUUpYkdc9umqmDilgSy5Z3Foafw+v4JJThfw68T/9G2gxZLrQTbAlvFPFfPM9Ehk6cY4+8WpY32uN8w5vrHyoSZGr343NxCUGIp6fQ9sSuOLMoJg7hNw="},
            {"client-side-encryption-cek-alg", "AES/CTR/NoPadding"},
            {"client-side-encryption-wrap-alg", "RSA/NONE/PKCS1Padding"},
        }},
        {"go-enc-example.jpg", {
            {"client-side-encryption-key", "F2L5QjyA2s85tPvaGdQ5EKnU/XN5dUWqZfgwcM4gfzPMcDWR93AZGSpeB9VSJBYPdIqhy1cevKEJv+Dv2ckDuDJ7nzijwcBnO5tPl5jXYlWxgzj6t1gMqQr/LENbB5iC8hzGkkoVWjWtSPDB+uE3+qf4V1A0308OqSM3OKxV0VI="},
            {"client-side-encryption-start", "D+3z6ftLp500eVnvsat5awYdYI/jTeSRlGlmHNrhTm3l1bonYP1v72vGqZhvOpT++9ZXOhdePu82gjhqVfh8Qv2HZsVGeJLzQJRU8kIKc7PRI4SoqpHZh2VYsASvnDtxVy2MQmpJzvG8xr4j3I29EgsEha7NV+2hGq/dolxLHNc="},
            {"client-side-encryption-cek-alg", "AES/CTR/NoPadding"},
            {"client-side-encryption-wrap-alg", "RSA/NONE/PKCS1Padding"},
        }},
    };

    std::string dataPath = TEST_DATA_PATH;
    std::string oriFile = dataPath + "example.jpg";

    auto provider = std::make_shared<StaticCredentialsProvider>(
            Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;

    auto compatMaster = std::make_shared<crypto::MasterRsaCipher>("", kCompatPrivateKey);
    crypto::EncryptionConfiguration compatEncConfig;
    compatEncConfig.masterCipher = compatMaster;
    OSSEncryptionClient compatClient(config, std::move(compatEncConfig));

    for (auto& tc : cases) {
        std::string encFilePath = dataPath + tc.encFile;
        std::ifstream encStream(encFilePath, std::ios::binary);
        ASSERT_TRUE(encStream.good()) << "Test data file not found: " << encFilePath;
        std::string encData((std::istreambuf_iterator<char>(encStream)),
                            std::istreambuf_iterator<char>());
        encStream.close();

        std::string key = "compat-" + tc.encFile;

        HeaderCollection meta;
        for (auto& [k, v] : tc.cseHeaders) {
            meta[k] = v;
        }

        auto putOutcome = plainClient_->putObject(
                models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString(encData))
                    .setMetadata(meta));
        ASSERT_TRUE(putOutcome.has_value()) << "upload failed for " << tc.encFile;

        auto getOutcome = compatClient.getObject(
                models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key));
        ASSERT_TRUE(getOutcome.has_value()) << "decrypt-get failed for " << tc.encFile;

        auto decrypted = readStream(*getOutcome.value().getBody());

        std::ifstream oriStream(oriFile, std::ios::binary);
        ASSERT_TRUE(oriStream.good()) << "Original data file not found: " << oriFile;
        std::string oriData((std::istreambuf_iterator<char>(oriStream)),
                            std::istreambuf_iterator<char>());
        oriStream.close();
        EXPECT_EQ(oriData, decrypted) << "content mismatch for " << tc.encFile;
    }
}

} // namespace oss2
} // namespace alibabacloud
