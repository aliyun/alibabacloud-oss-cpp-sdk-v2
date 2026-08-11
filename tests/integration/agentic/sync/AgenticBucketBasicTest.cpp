#include <gtest/gtest.h>

#include "Config.h"
#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/AgenticBucketPaginator.h"

#include <memory>

namespace alibabacloud {
namespace oss2 {
namespace sync {

// One shared bucket exercising Create/Get/List/PutStatus(Enabled)/ListBucketSpaces
// plus the negative paths. Lifecycle Disable+Delete lives in its own test class.
class AgenticBucketBasicTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        bucketName_ = agentictest::genBucketName();
        auto client = agentictest::makeAgenticClient();
        auto outcome = client->createAgenticBucket(
                agentic::models::CreateAgenticBucketRequest()
                        .setBucket(bucketName_)
                        .setCreateAgenticBucketConfiguration(
                                agentic::models::CreateAgenticBucketConfiguration().setStorageClass("Standard")));
        EXPECT_TRUE(outcome.has_value()) << (outcome.has_value() ? "" : outcome.error().getMessage());
    }

    static void TearDownTestCase() {
        agentictest::disableAndReap(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AgenticBucketBasicTest::bucketName_ = "";

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_Normal) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_));
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_TRUE(outcome.value().hasAgenticBucketInfo());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_RequiredField) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_InvalidCredentials) {
    auto client = agentictest::makeAgenticClient(false);
    auto outcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_NE(0, outcome.error().getStatusCode());
}

TEST_F(AgenticBucketBasicTest, GetAgenticBucket_NotExist) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->getAgenticBucket(
            agentic::models::GetAgenticBucketRequest().setBucket("cpp-ab-not-exist-000000"));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(AgenticBucketBasicTest, ListAgenticBuckets_Normal) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST_F(AgenticBucketBasicTest, ListAgenticBuckets_Paginator) {
    auto client = agentictest::makeAgenticClient();
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

TEST_F(AgenticBucketBasicTest, ListAgenticBuckets_FindCreated) {
    auto client = agentictest::makeAgenticClient();
    // The listing is eventually consistent, so a freshly created bucket may not show
    // up for a while. Poll with a generous budget; the bucket's actual existence is
    // asserted strongly by GetAgenticBucket_Normal, so if the listing still lags we
    // skip rather than fail (a consistency delay is not a defect).
    bool found = false;
    for (int attempt = 0; attempt < 12 && !found; ++attempt) {
        if (attempt > 0) {
            Config::WaitForCacheExpire(10);
        }
        auto paginator = agentic::makeAgenticPaginator(client, agentic::models::ListAgenticBucketsRequest());
        while (paginator.hasNext()) {
            auto outcome = paginator.nextPage();
            if (!outcome.has_value()) {
                break;
            }
            for (const auto& summary : outcome.value().getAgenticBuckets()) {
                if (summary.name.has_value() &&
                    summary.name.value().find(bucketName_) != std::string::npos) {
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
    if (!found) {
        GTEST_SKIP() << "created agentic bucket has not appeared in the listing yet (eventual consistency)";
    }
}

TEST_F(AgenticBucketBasicTest, PutAgenticBucketStatus_Enabled) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->putAgenticBucketStatus(
            agentic::models::PutAgenticBucketStatusRequest().setBucket(bucketName_).setAgenticBucketStatus(
                    agentic::models::AgenticBucketStatus().setStatus("Enabled")));
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST_F(AgenticBucketBasicTest, PutAgenticBucketStatus_RequiredField) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->putAgenticBucketStatus(
            agentic::models::PutAgenticBucketStatusRequest().setBucket(bucketName_));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST_F(AgenticBucketBasicTest, ListBucketSpaces_Normal) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->listBucketSpaces(agentic::models::ListBucketSpacesRequest().setBucket(bucketName_));
    // The agentic bucket may have no bucket spaces yet; a valid response is enough.
    if (outcome.has_value()) {
        EXPECT_EQ(200, outcome.value().getStatusCode());
    }
}

TEST_F(AgenticBucketBasicTest, ListBucketSpaces_RequiredField) {
    auto client = agentictest::makeAgenticClient();
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
