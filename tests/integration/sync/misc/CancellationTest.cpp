#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace sync {

class CancellationTest : public ::testing::Test {
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

std::string CancellationTest::bucketName_ = "";

TEST_F(CancellationTest, PutObject_AlreadyCancelled) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();
    cts->cancel();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("cancel-already")
            .setBody(RequestBody::fromString("data")),
        &opts);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST_F(CancellationTest, PutObject_CancelDuringTransfer) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();

    // 4MB body + 30KB/s (245760 bit/s) rate limit->~136s transfer time
    std::string body(4 * 1024 * 1024, 'A');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("cancel-during")
        .setBody(RequestBody::fromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::thread canceller([&cts]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        cts->cancel();
    });

    auto outcome = client->putObject(request, &opts);
    canceller.join();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST_F(CancellationTest, PutObject_TimeoutViaCancelAfter) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::seconds(2));

    // 4MB body + 30KB/s rate limit->transfer takes much longer than 2s
    std::string body(4 * 1024 * 1024, 'B');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("cancel-timeout")
        .setBody(RequestBody::fromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client->putObject(request, &opts);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
