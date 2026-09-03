#include <gtest/gtest.h>

#include "Config.h"
#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"

#include <future>
#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace async {

static std::shared_ptr<agentic::OSSAsyncAgenticBucketClient> makeAsyncAgenticClient(bool valid = true) {
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.accountId = Config::AccountID;
    if (valid) {
        config.credentialsProvider =
                std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    } else {
        config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("invalid-ak", "invalid-sk");
    }
    return std::make_shared<agentic::OSSAsyncAgenticBucketClient>(config);
}

class AsyncAgenticBucketBasicTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        // Share the reaper-matched prefix so this bucket's backlog is bounded too.
        bucketName_ = agentictest::genBucketName();

        auto client = makeAsyncAgenticClient();
        std::promise<agentic::CreateAgenticBucketOutcome> p;
        client->createAgenticBucketAsync(
                agentic::models::CreateAgenticBucketRequest()
                        .setBucket(bucketName_)
                        .setCreateAgenticBucketConfiguration(
                                agentic::models::CreateAgenticBucketConfiguration().setStorageClass("Standard")),
                [&p](agentic::CreateAgenticBucketOutcome o) { p.set_value(std::move(o)); });
        auto outcome = p.get_future().get();
        EXPECT_TRUE(outcome.has_value()) << (outcome.has_value() ? "" : outcome.error().getMessage());
    }

    static void TearDownTestCase() {
        // Disable this run's bucket and reap ready backlog from earlier runs.
        agentictest::disableAndReap(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncAgenticBucketBasicTest::bucketName_ = "";

TEST_F(AsyncAgenticBucketBasicTest, GetAgenticBucketAsync_Normal) {
    auto client = makeAsyncAgenticClient();
    std::promise<agentic::GetAgenticBucketOutcome> p;
    client->getAgenticBucketAsync(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_),
                                  [&p](agentic::GetAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_TRUE(outcome.value().hasAgenticBucketInfo());
}

TEST_F(AsyncAgenticBucketBasicTest, GetAgenticBucketAsync_RequiredField) {
    auto client = makeAsyncAgenticClient();
    std::promise<agentic::GetAgenticBucketOutcome> p;
    client->getAgenticBucketAsync(agentic::models::GetAgenticBucketRequest(),
                                  [&p](agentic::GetAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AsyncAgenticBucketBasicTest, GetAgenticBucketAsync_InvalidCredentials) {
    auto client = makeAsyncAgenticClient(false);
    std::promise<agentic::GetAgenticBucketOutcome> p;
    client->getAgenticBucketAsync(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_),
                                  [&p](agentic::GetAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_NE(0, outcome.error().getStatusCode());
}

TEST_F(AsyncAgenticBucketBasicTest, ListAgenticBucketsAsync_Normal) {
    auto client = makeAsyncAgenticClient();
    std::promise<agentic::ListAgenticBucketsOutcome> p;
    client->listAgenticBucketsAsync(agentic::models::ListAgenticBucketsRequest(),
                                    [&p](agentic::ListAgenticBucketsOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST_F(AsyncAgenticBucketBasicTest, ListBucketSpacesAsync_RequiredField) {
    auto client = makeAsyncAgenticClient();
    std::promise<agentic::ListBucketSpacesOutcome> p;
    client->listBucketSpacesAsync(agentic::models::ListBucketSpacesRequest(),
                                  [&p](agentic::ListBucketSpacesOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
