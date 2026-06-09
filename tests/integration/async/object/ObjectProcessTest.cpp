#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"

#include <condition_variable>
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

class AsyncObjectProcessTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());

        std::string filePath = TEST_DATA_PATH "example.jpg";
        std::ifstream ifs(filePath, std::ios::binary);
        ASSERT_TRUE(ifs.good());
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        auto putFuture = client->asyncCall(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(imageKey_)
                .setBody(RequestBody::fromString(content)));
        EXPECT_TRUE(putFuture.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
    static std::string imageKey_;
};

std::string AsyncObjectProcessTest::bucketName_ = "";
std::string AsyncObjectProcessTest::imageKey_ = "test-async-process-source.jpg";

TEST_F(AsyncObjectProcessTest, ProcessObjectAsync_ImageResize) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-async-process-resize-target.jpg";
    std::string process = "image/resize,w_100|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto ar = std::make_shared<AsyncResult<ProcessObjectOutcome>>();
    client->processObjectAsync(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process),
        [ar](ProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    const auto& body = outcome.value().getBody();
    EXPECT_FALSE(body.empty());
    EXPECT_NE(body.find("\"bucket\""), std::string::npos);
    EXPECT_NE(body.find("\"object\""), std::string::npos);
    EXPECT_NE(body.find("\"status\""), std::string::npos);
    EXPECT_NE(body.find(targetKey), std::string::npos);

    auto headFuture = client->asyncCall(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(targetKey));
    auto headOutcome = headFuture.get();
    EXPECT_TRUE(headOutcome.has_value());
}

TEST_F(AsyncObjectProcessTest, ProcessObjectAsync_InvalidProcess) {
    auto client = ClientHelper::GetDefaultClient();

    auto ar = std::make_shared<AsyncResult<ProcessObjectOutcome>>();
    client->processObjectAsync(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess("invalid/process"),
        [ar](ProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(AsyncObjectProcessTest, ProcessObjectAsync_KeyNotExist) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-async-process-noexist-target.jpg";
    std::string process = "image/resize,w_100|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto ar = std::make_shared<AsyncResult<ProcessObjectOutcome>>();
    client->processObjectAsync(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey("no-such-key-async-process.jpg")
            .setProcess(process),
        [ar](ProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(AsyncObjectProcessTest, ProcessObjectAsync_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    auto ar = std::make_shared<AsyncResult<ProcessObjectOutcome>>();
    client->processObjectAsync(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess("image/resize,w_100"),
        [ar](ProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(AsyncObjectProcessTest, AsyncProcessObjectAsync_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-async-process-video-target.mp4";
    std::string process = "video/convert,f_mp4|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto ar = std::make_shared<AsyncResult<AsyncProcessObjectOutcome>>();
    client->asyncProcessObjectAsync(
        models::AsyncProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process),
        [ar](AsyncProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    if (outcome.has_value()) {
        const auto& body = outcome.value().getBody();
        EXPECT_FALSE(body.empty());
        EXPECT_NE(body.find("\"EventId\""), std::string::npos);
        EXPECT_NE(body.find("\"TaskId\""), std::string::npos);
        EXPECT_NE(body.find("\"RequestId\""), std::string::npos);
    }
}

TEST_F(AsyncObjectProcessTest, AsyncProcessObjectAsync_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    auto ar = std::make_shared<AsyncResult<AsyncProcessObjectOutcome>>();
    client->asyncProcessObjectAsync(
        models::AsyncProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess("video/convert,f_mp4"),
        [ar](AsyncProcessObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
