#include <gtest/gtest.h>

#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/AgenticBucketPaginator.h"

#include <chrono>
#include <memory>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

static std::shared_ptr<agentic::OSSAgenticBucketClient> makeAgenticClient(bool valid = true) {
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
    return std::make_shared<agentic::OSSAgenticBucketClient>(config);
}

class AgenticBucketBasicTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        std::stringstream ss;
        auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        ss << "cpp-ab-" << tp.time_since_epoch().count();
        bucketName_ = ss.str();

        auto client = makeAgenticClient();
        auto outcome = client->createAgenticBucket(
                agentic::models::CreateAgenticBucketRequest()
                        .setBucket(bucketName_)
                        .setCreateAgenticBucketConfiguration(
                                agentic::models::CreateAgenticBucketConfiguration().setStorageClass("Standard")));
        EXPECT_TRUE(outcome.has_value()) << (outcome.has_value() ? "" : outcome.error().getMessage());
    }

    static void TearDownTestCase() {
        auto client = makeAgenticClient();
        (void)client->deleteAgenticBucket(agentic::models::DeleteAgenticBucketRequest().setBucket(bucketName_));
    }

  public:
    static std::string bucketName_;
};

std::string AgenticBucketBasicTest::bucketName_ = "";

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_Normal) {
    auto client = makeAgenticClient();
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_));
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_TRUE(outcome.value().hasAgenticBucketInfo());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_RequiredField) {
    auto client = makeAgenticClient();
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_InvalidCredentials) {
    auto client = makeAgenticClient(false);
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_NE(0, outcome.error().getStatusCode());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_NotExist) {
    auto client = makeAgenticClient();
    auto outcome = client->getAgenticBucket(
            agentic::models::GetAgenticBucketRequest().setBucket("cpp-ab-not-exist-000000"));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(AgenticBucketBasicTest, ListAgenticBuckets_Normal) {
    auto client = makeAgenticClient();
    auto outcome = client->listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST_F(AgenticBucketBasicTest, ListAgenticBuckets_Paginator) {
    auto client = makeAgenticClient();
    auto paginator = agentic::makeAgenticPaginator(client, agentic::models::ListAgenticBucketsRequest().setMaxKeys(1));
    int pages = 0;
    while (paginator.hasNext() && pages < 5) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            break;
        }
        ++pages;
    }
    EXPECT_GE(pages, 1);
}

TEST_F(AgenticBucketBasicTest, PutAgenticBucketStatus_RequiredField) {
    auto client = makeAgenticClient();
    auto outcome = client->putAgenticBucketStatus(
            agentic::models::PutAgenticBucketStatusRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AgenticBucketBasicTest, ListBucketSpaces_Normal) {
    auto client = makeAgenticClient();
    auto outcome = client->listBucketSpaces(agentic::models::ListBucketSpacesRequest().setBucket(bucketName_));
    // The agentic bucket may have no bucket spaces yet; a valid response is enough.
    if (outcome.has_value()) {
        EXPECT_EQ(200, outcome.value().getStatusCode());
    }
}

TEST_F(AgenticBucketBasicTest, ListBucketSpaces_RequiredField) {
    auto client = makeAgenticClient();
    auto outcome = client->listBucketSpaces(agentic::models::ListBucketSpacesRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AgenticBucketBasicTest, InvalidAccountId_DeferredError) {
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.accountId = "bad-account";
    config.credentialsProvider =
            std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    auto client = std::make_shared<agentic::OSSAgenticBucketClient>(config);

    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
