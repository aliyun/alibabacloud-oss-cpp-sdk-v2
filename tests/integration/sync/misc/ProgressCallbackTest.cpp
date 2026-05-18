#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ProgressCallbackTest : public ::testing::Test {
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

std::string ProgressCallbackTest::bucketName_ = "";

TEST_F(ProgressCallbackTest, PutObject_Progress) {
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

    auto outcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("progress-test-put")
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    EXPECT_TRUE(outcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);
}

TEST_F(ProgressCallbackTest, UploadPart_Progress) {
    auto client = ClientHelper::GetDefaultClient();

    std::string key = "progress-test-multipart";

    auto initOutcome = client->initiateMultipartUpload(
            models::InitiateMultipartUploadRequest()
                    .setBucket(bucketName_)
                    .setKey(key));
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

    auto uploadOutcome = client->uploadPart(
            models::UploadPartRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setPartNumber(1)
                    .setUploadId(uploadId)
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    EXPECT_TRUE(uploadOutcome.has_value());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(data.size(), lastTransferred);
    EXPECT_EQ(data.size(), totalIncrement);

    // Cleanup
    (void)client->abortMultipartUpload(
            models::AbortMultipartUploadRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setUploadId(uploadId));
}

TEST_F(ProgressCallbackTest, PutObject_Progress_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    std::string data(1024, 'Z');
    int callCount = 0;

    ProgressCallback progress;
    progress.callback = [&](std::size_t, std::size_t, std::int64_t, std::uintptr_t) {
        callCount++;
    };

    auto outcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("progress-test-fail")
                    .setBody(RequestBody::FromString(data))
                    .setProgressCallback(progress));

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
