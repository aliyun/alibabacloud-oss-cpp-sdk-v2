#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectExtensionTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string ObjectExtensionTest::bucketName_ = "";

// --- putObjectFromFile ---

TEST_F(ObjectExtensionTest, PutObjectFromFile_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-put-from-file";
    std::string content = "hello put object from file";

    auto filePath = std::string(std::tmpnam(nullptr));
    {
        std::ofstream f(filePath, std::ios::binary);
        f << content;
    }

    auto outcome = client->putObjectFromFile(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    auto getOutcome = client->getObject(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(getOutcome.has_value());

    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());

    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, PutObjectFromFile_LargeFile) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-put-from-file-large";
    std::string content(512 * 1024 + 123, 'A');

    auto filePath = std::string(std::tmpnam(nullptr));
    {
        std::ofstream f(filePath, std::ios::binary);
        f << content;
    }

    auto outcome = client->putObjectFromFile(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::remove(filePath.c_str());
}

// --- getObjectToFile ---

TEST_F(ObjectExtensionTest, GetObjectToFile_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-to-file";
    std::string content = "hello get object to file";

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString(content)));

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_LargeObject) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-to-file-large";
    std::string content(3 * 1024 * 1024 + 1234, 'B');

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString(content)));

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_NotExist) {
    auto client = ClientHelper::GetDefaultClient();

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey("no-such-key-ext-test"),
            filePath);
    EXPECT_FALSE(outcome.has_value());
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_TruncatesExisting) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-to-file-truncate";
    std::string content = "short content";

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString(content)));

    auto filePath = std::string(std::tmpnam(nullptr));
    {
        std::ofstream f(filePath, std::ios::binary);
        f << std::string(4096, 'Z');
    }

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_WithRange) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-to-file-range";
    std::string content = "0123456789abcdefghij";

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString(content)));

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key).setRange("bytes=5-14"),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ("56789abcde", downloaded);
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_WithOpenEndRange) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-get-to-file-range-open";
    std::string content = "0123456789abcdefghij";

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString(content)));

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key).setRange("bytes=10-"),
            filePath);
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ("abcdefghij", downloaded);
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_InvalidRange) {
    auto client = ClientHelper::GetDefaultClient();

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey("any-key").setRange("invalid-range"),
            filePath);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentInvalid", outcome.error().getCode());
    std::remove(filePath.c_str());
}

TEST_F(ObjectExtensionTest, GetObjectToFile_MultiRangeRejected) {
    auto client = ClientHelper::GetDefaultClient();

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client->getObjectToFile(
            models::GetObjectRequest().setBucket(bucketName_).setKey("any-key").setRange("bytes=0-10,20-30"),
            filePath);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentInvalid", outcome.error().getCode());
    std::remove(filePath.c_str());
}

// --- isObjectExist ---

TEST_F(ObjectExtensionTest, IsObjectExist_True) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-is-object-exist";

    client->putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::FromString("data")));

    auto outcome = client->isObjectExist(bucketName_, key);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST_F(ObjectExtensionTest, IsObjectExist_False) {
    auto client = ClientHelper::GetDefaultClient();

    auto outcome = client->isObjectExist(bucketName_, "no-such-key-ext-test");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST_F(ObjectExtensionTest, IsObjectExist_BucketNotExist) {
    auto client = ClientHelper::GetDefaultClient();

    auto outcome = client->isObjectExist("no-such-bucket-ext-test-xyz", "key");
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchBucket", outcome.error().getCode());
}

TEST_F(ObjectExtensionTest, IsObjectExist_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    auto outcome = client->isObjectExist(bucketName_, "key");
    EXPECT_FALSE(outcome.has_value());
}

// --- isBucketExist ---

TEST_F(ObjectExtensionTest, IsBucketExist_True) {
    auto client = ClientHelper::GetDefaultClient();

    auto outcome = client->isBucketExist(bucketName_);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST_F(ObjectExtensionTest, IsBucketExist_False) {
    auto client = ClientHelper::GetDefaultClient();

    auto outcome = client->isBucketExist("no-such-bucket-ext-test-xyz");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST_F(ObjectExtensionTest, IsBucketExist_NoPermission_StillTrue) {
    auto client = ClientHelper::GetInvalidClient();

    auto outcome = client->isBucketExist(bucketName_);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
