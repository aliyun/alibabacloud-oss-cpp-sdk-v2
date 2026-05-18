#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncProgressCallbackTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncProgressCallbackTest::bucketName_ = "";

TEST_F(AsyncProgressCallbackTest, PutObject_Progress) {
    auto client = ClientHelper::GetDefaultClient();

    std::string data(128 * 1024, 'X');
    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;

    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(data.size()), total);
    };

    auto future = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("async-progress-test-put")
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);
}

TEST_F(AsyncProgressCallbackTest, UploadPart_Progress) {
    auto client = ClientHelper::GetDefaultClient();

    std::string key = "async-progress-test-multipart";

    auto initFuture = client->asyncCall(
            models::InitiateMultipartUploadRequest()
                    .setBucket(bucketName_)
                    .setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    auto uploadId = initOutcome.value().getUploadId();

    std::string data(256 * 1024, 'Y');
    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;

    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(data.size()), total);
    };

    auto uploadFuture = client->asyncCall(
            models::UploadPartRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setPartNumber(1)
                    .setUploadId(uploadId)
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    auto uploadOutcome = uploadFuture.get();
    EXPECT_TRUE(uploadOutcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);

    // Cleanup
    (void)client->asyncCall(
            models::AbortMultipartUploadRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setUploadId(uploadId)).get();
}

TEST_F(AsyncProgressCallbackTest, PutObject_Progress_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    std::string data(1024, 'Z');
    int callCount = 0;

    ProgressCallback progress;
    progress.callback = [&](std::size_t, std::size_t, std::int64_t, std::uintptr_t) {
        callCount++;
    };

    auto future = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("async-progress-test-fail")
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
