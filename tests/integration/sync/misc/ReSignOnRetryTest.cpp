#include <gtest/gtest.h>

#include "Config.h"
#include "ReSignProbes.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "src/transport/HttpTransportFactory.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ReSignOnRetryTest : public ::testing::Test {
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

    static OSSClient MakeClient(const std::shared_ptr<HttpTransport>& transport) {
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider =
            std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
        config.httpTransport = transport;
        // a delay above one second puts the retry in a different signing second
        config.retryer = std::make_shared<StandardRetryer>(
            3, std::make_unique<FixedDelayBackoff>(std::chrono::milliseconds(1500)));
        return OSSClient(config);
    }

  public:
    static std::string bucketName_;
};

std::string ReSignOnRetryTest::bucketName_ = "";

TEST_F(ReSignOnRetryTest, PutObject_ReSignsRetriedRequest) {
    auto transport = std::make_shared<test::CapturingHttpTransport>(transport::HttpTransportFactory::create({}));
    auto client = MakeClient(transport);

    std::string key = "test-resign-on-retry";
    std::string content(64 * 1024, 'A');
    auto body = std::make_shared<test::TruncateFirstAttemptContent>(content);

    auto outcome = client.putObject(
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));

    ASSERT_TRUE(outcome.has_value()) << outcome.error().getCode() << ": " << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(2, body->getSpanCount());

    auto sent = transport->getSentHeaders();
    ASSERT_EQ(2U, sent.size());
    EXPECT_NE(sent[0].at("x-oss-date"), sent[1].at("x-oss-date"));
    EXPECT_NE(sent[0].at("Date"), sent[1].at("Date"));
    EXPECT_NE(sent[0].at("Authorization"), sent[1].at("Authorization"));

    auto getOutcome = client.getObject(models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    ASSERT_TRUE(getOutcome.has_value());
    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
