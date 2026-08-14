#include <gtest/gtest.h>

#include "Config.h"
#include "ReSignProbes.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "src/transport/HttpTransportFactory.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncReSignOnRetryTest : public ::testing::Test {
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

    static std::shared_ptr<OSSAsyncClient> MakeClient(const std::shared_ptr<AsyncHttpTransport>& transport) {
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider =
            std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
        config.asyncHttpTransport = transport;
        // a delay above one second puts the retry in a different signing second
        config.retryer = std::make_shared<StandardRetryer>(
            3, std::make_unique<FixedDelayBackoff>(std::chrono::milliseconds(1500)));
        return std::make_shared<OSSAsyncClient>(config);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncReSignOnRetryTest::bucketName_ = "";

TEST_F(AsyncReSignOnRetryTest, PutObject_ReSignsRetriedRequest) {
    auto transport =
        std::make_shared<test::CapturingAsyncHttpTransport>(transport::AsyncHttpTransportFactory::create({}));
    auto client = MakeClient(transport);

    std::string key = "test-resign-on-retry";
    std::string content(64 * 1024, 'A');
    auto body = std::make_shared<test::TruncateFirstAttemptContent>(content);

    auto future = client->asyncCall(
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    auto outcome = future.get();

    ASSERT_TRUE(outcome.has_value()) << outcome.error().getCode() << ": " << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(2, body->getSpanCount());

    auto sent = transport->getSentHeaders();
    ASSERT_EQ(2U, sent.size());
    EXPECT_NE(sent[0].at("x-oss-date"), sent[1].at("x-oss-date"));
    EXPECT_NE(sent[0].at("Date"), sent[1].at("Date"));
    EXPECT_NE(sent[0].at("Authorization"), sent[1].at("Authorization"));

    auto getFuture = client->asyncCall(models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getFuture.get();
    ASSERT_TRUE(getOutcome.has_value());
    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
