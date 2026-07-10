#include <gtest/gtest.h>

#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"

#include <chrono>
#include <future>
#include <memory>
#include <sstream>

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
        std::stringstream ss;
        auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        ss << "cpp-abasync-" << tp.time_since_epoch().count();
        bucketName_ = ss.str();

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
        auto client = makeAsyncAgenticClient();
        std::promise<agentic::DeleteAgenticBucketOutcome> p;
        client->deleteAgenticBucketAsync(agentic::models::DeleteAgenticBucketRequest().setBucket(bucketName_),
                                         [&p](agentic::DeleteAgenticBucketOutcome o) { p.set_value(std::move(o)); });
        (void)p.get_future().get();
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
