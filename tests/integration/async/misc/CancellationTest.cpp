#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncCancellationTest : public ::testing::Test {
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

std::string AsyncCancellationTest::bucketName_ = "";

TEST_F(AsyncCancellationTest, PutObject_AlreadyCancelled) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();
    cts->cancel();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("cancel-already")
            .setBody(RequestBody::FromString("data")),
        &opts);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST_F(AsyncCancellationTest, PutObject_CancelDuringTransfer) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();

    // 4MB body + 30KB/s (245760 bit/s) rate limit->~136s transfer time
    std::string body(4 * 1024 * 1024, 'A');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("cancel-during")
        .setBody(RequestBody::FromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client->asyncCall(request, &opts);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    cts->cancel();

    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST_F(AsyncCancellationTest, PutObject_TimeoutViaCancelAfter) {
    auto client = ClientHelper::GetDefaultClient();
    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::seconds(2));

    // 4MB body + 30KB/s rate limit->transfer takes much longer than 2s
    std::string body(4 * 1024 * 1024, 'B');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("cancel-timeout")
        .setBody(RequestBody::FromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client->asyncCall(request, &opts);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ("Request canceled by CancellationToken", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
