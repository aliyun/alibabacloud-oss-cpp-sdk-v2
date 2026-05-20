#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"

#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace sync {

static std::shared_ptr<OSSClient> makeClientWithExecutor() {
    auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;
    config.executor = std::make_shared<DefaultExecutor>();
    return std::make_shared<OSSClient>(config);
}

class DisableRequestProcessingTest : public ::testing::Test {
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

std::string DisableRequestProcessingTest::bucketName_ = "";

TEST_F(DisableRequestProcessingTest, DisableBeforeRequest) {
    auto client = ClientHelper::GetDefaultClient();
    client->disableRequest();

    auto outcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("disable-before")
            .setBody(RequestBody::FromString("data")));

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client->enableRequest();
}

TEST_F(DisableRequestProcessingTest, DisableDuringTransfer) {
    auto client = ClientHelper::GetDefaultClient();

    // 4MB body + 30KB/s (245760 bit/s) rate limit -> ~136s transfer time
    std::string body(4 * 1024 * 1024, 'A');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("disable-during")
        .setBody(RequestBody::FromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    std::thread disabler([&client]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        client->disableRequest();
    });

    auto outcome = client->putObject(request);
    disabler.join();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client->enableRequest();
}

TEST_F(DisableRequestProcessingTest, EnableAfterDisable_FullCycle) {
    auto client = ClientHelper::GetDefaultClient();

    client->disableRequest();

    auto outcome1 = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("disable-cycle")
            .setBody(RequestBody::FromString("data")));
    EXPECT_FALSE(outcome1.has_value());
    EXPECT_EQ(outcome1.error().getErrorCode(), ErrorCondition::Canceled);

    client->enableRequest();

    auto outcome2 = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("disable-cycle")
            .setBody(RequestBody::FromString("data")));
    EXPECT_TRUE(outcome2.has_value());
}

TEST_F(DisableRequestProcessingTest, AsyncDisableBeforeRequest) {
    auto client = makeClientWithExecutor();
    client->disableRequest();

    auto future = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("async-disable-before")
            .setBody(RequestBody::FromString("data")));
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client->enableRequest();
}

TEST_F(DisableRequestProcessingTest, AsyncDisableDuringTransfer) {
    auto client = makeClientWithExecutor();

    // 4MB body + 30KB/s (245760 bit/s) rate limit -> ~136s transfer time
    std::string body(4 * 1024 * 1024, 'A');
    auto request = models::PutObjectRequest()
        .setBucket(bucketName_)
        .setKey("async-disable-during")
        .setBody(RequestBody::FromString(body));
    request.addHeader("x-oss-traffic-limit", "245760");

    auto future = client->asyncCall(request);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    client->disableRequest();

    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client->enableRequest();
}

TEST_F(DisableRequestProcessingTest, AsyncDisableCancelsBatchRequests) {
    auto client = makeClientWithExecutor();

    // 4MB body + 30KB/s rate limit per request
    std::string body(4 * 1024 * 1024, 'A');

    std::vector<std::future<PutObjectOutcome>> futures;
    for (int i = 0; i < 3; i++) {
        auto request = models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey("async-batch-" + std::to_string(i))
            .setBody(RequestBody::FromString(body));
        request.addHeader("x-oss-traffic-limit", "245760");
        futures.push_back(client->asyncCall(request));
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    client->disableRequest();

    for (auto& f : futures) {
        auto outcome = f.get();
        EXPECT_FALSE(outcome.has_value());
        EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
    }

    client->enableRequest();
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
