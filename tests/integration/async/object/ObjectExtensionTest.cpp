#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace async {

namespace {

template<typename T>
struct AsyncResult {
    std::mutex mu;
    std::condition_variable cv;
    std::optional<T> result;

    void set(T val) {
        std::lock_guard<std::mutex> lock(mu);
        result = std::move(val);
        cv.notify_one();
    }

    T wait() {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [this]{ return result.has_value(); });
        return std::move(*result);
    }
};

} // namespace

class AsyncObjectExtensionTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
        Config::CleanTempDir();
    }

  public:
    static std::string bucketName_;
};

std::string AsyncObjectExtensionTest::bucketName_ = "";

// --- putObjectFromFileAsync ---

TEST_F(AsyncObjectExtensionTest, PutObjectFromFileAsync_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-put-from-file";
    std::string content = "hello async put object from file";

    auto filePath = Config::GenRandomFileName();
    {
        std::ofstream f(filePath, std::ios::binary);
        f << content;
    }

    auto ar = std::make_shared<AsyncResult<PutObjectOutcome>>();
    client->putObjectFromFileAsync(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key),
            filePath,
            [ar](PutObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    auto getFuture = client->asyncCall(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getFuture.get();
    EXPECT_TRUE(getOutcome.has_value());

    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());

    std::remove(filePath.c_str());
}

TEST_F(AsyncObjectExtensionTest, PutObjectFromFileAsync_LargeFile) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-put-from-file-large";
    std::string content(512 * 1024 + 123, 'A');

    auto filePath = Config::GenRandomFileName();
    {
        std::ofstream f(filePath, std::ios::binary);
        f << content;
    }

    auto ar = std::make_shared<AsyncResult<PutObjectOutcome>>();
    client->putObjectFromFileAsync(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key),
            filePath,
            [ar](PutObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::remove(filePath.c_str());
}

// --- getObjectToFileAsync ---

TEST_F(AsyncObjectExtensionTest, GetObjectToFileAsync_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-get-to-file";
    std::string content = "hello async get object to file";

    auto putFuture = client->asyncCall(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::fromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto filePath = Config::GenRandomFileName();

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client->getObjectToFileAsync(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST_F(AsyncObjectExtensionTest, GetObjectToFileAsync_LargeObject) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-get-to-file-large";
    std::string content(3 * 1024 * 1024 + 1234, 'B');

    auto putFuture = client->asyncCall(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::fromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto filePath = Config::GenRandomFileName();

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client->getObjectToFileAsync(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST_F(AsyncObjectExtensionTest, GetObjectToFileAsync_NotExist) {
    auto client = ClientHelper::GetDefaultClient();

    auto filePath = Config::GenRandomFileName();

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client->getObjectToFileAsync(
            models::GetObjectRequest().setBucket(bucketName_).setKey("no-such-key-async-ext"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
    std::remove(filePath.c_str());
}

TEST_F(AsyncObjectExtensionTest, GetObjectToFileAsync_TruncatesExisting) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-get-to-file-truncate";
    std::string content = "short content";

    auto putFuture = client->asyncCall(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::fromString(content)));
    EXPECT_TRUE(putFuture.get().has_value());

    auto filePath = Config::GenRandomFileName();
    {
        std::ofstream f(filePath, std::ios::binary);
        f << std::string(4096, 'Z');
    }

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client->getObjectToFileAsync(
            models::GetObjectRequest().setBucket(bucketName_).setKey(key),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

// --- isObjectExistAsync ---

TEST_F(AsyncObjectExtensionTest, IsObjectExistAsync_True) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-async-is-object-exist";

    auto putFuture = client->asyncCall(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key)
                    .setBody(RequestBody::fromString("data")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isObjectExistAsync(bucketName_, key,
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST_F(AsyncObjectExtensionTest, IsObjectExistAsync_False) {
    auto client = ClientHelper::GetDefaultClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isObjectExistAsync(bucketName_, "no-such-key-async-ext",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST_F(AsyncObjectExtensionTest, IsObjectExistAsync_BucketNotExist) {
    auto client = ClientHelper::GetDefaultClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isObjectExistAsync("no-such-bucket-async-ext-xyz", "key",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchBucket", outcome.error().getCode());
}

TEST_F(AsyncObjectExtensionTest, IsObjectExistAsync_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isObjectExistAsync(bucketName_, "key",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
}

// --- isBucketExistAsync ---

TEST_F(AsyncObjectExtensionTest, IsBucketExistAsync_True) {
    auto client = ClientHelper::GetDefaultClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isBucketExistAsync(bucketName_,
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST_F(AsyncObjectExtensionTest, IsBucketExistAsync_False) {
    auto client = ClientHelper::GetDefaultClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isBucketExistAsync("no-such-bucket-async-ext-xyz",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST_F(AsyncObjectExtensionTest, IsBucketExistAsync_NoPermission_StillTrue) {
    auto client = ClientHelper::GetInvalidClient();

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client->isBucketExistAsync(bucketName_,
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
