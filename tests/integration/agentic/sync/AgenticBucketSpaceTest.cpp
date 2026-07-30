#include <gtest/gtest.h>

#include "Config.h"
#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/BucketSpaceHelper.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

// ListBucketSpaces, bucket/object interfaces via the bucket space client, and the
// BucketSpaceHelper name builder driving a plain OSSClient, over one shared bucket
// space. Creating the space requires the parent agentic bucket full name.
class AgenticBucketSpaceTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        bucketName_ = agentictest::genBucketName();
        auto client = agentictest::makeAgenticClient();
        auto createOutcome =
                client->createAgenticBucket(agentic::models::CreateAgenticBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(createOutcome.has_value()) << (createOutcome.has_value() ? "" : createOutcome.error().getMessage());

        auto bsClient = agentictest::makeBsClient();
        auto putBucketOutcome = bsClient.putBucket(
                models::PutBucketRequest().setBucket(bucketName_).setAgenticBucket(
                        agentictest::buildFullName(bucketName_, "ab-apsr")));
        EXPECT_TRUE(putBucketOutcome.has_value())
                << (putBucketOutcome.has_value() ? "" : putBucketOutcome.error().getMessage());
    }

    static void TearDownTestCase() {
        auto bsClient = agentictest::makeBsClient();
        (void)bsClient.deleteBucket(models::DeleteBucketRequest().setBucket(bucketName_));
        agentictest::disableAndReap(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AgenticBucketSpaceTest::bucketName_ = "";

TEST_F(AgenticBucketSpaceTest, ListBucketSpaces) {
    auto client = agentictest::makeAgenticClient();
    auto outcome = client->listBucketSpaces(agentic::models::ListBucketSpacesRequest().setBucket(bucketName_));
    ASSERT_TRUE(outcome.has_value()) << outcome.error().getMessage();
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST_F(AgenticBucketSpaceTest, BucketLifecycle) {
    auto bsClient = agentictest::makeBsClient();

    auto putAclOutcome = bsClient.putBucketAcl(models::PutBucketAclRequest().setBucket(bucketName_).setAcl("private"));
    ASSERT_TRUE(putAclOutcome.has_value()) << putAclOutcome.error().getMessage();

    auto getAclOutcome = bsClient.getBucketAcl(models::GetBucketAclRequest().setBucket(bucketName_));
    ASSERT_TRUE(getAclOutcome.has_value()) << getAclOutcome.error().getMessage();
    ASSERT_TRUE(getAclOutcome.value().getAccessControlPolicy().accessControlList.has_value());
    EXPECT_EQ("private", getAclOutcome.value().getAccessControlPolicy().accessControlList.value().grant);
}

TEST_F(AgenticBucketSpaceTest, ObjectLifecycle) {
    auto bsClient = agentictest::makeBsClient();
    auto key = "cpp-sdk-test-object-" + agentictest::randStr(6);
    const std::string content = "hello world";

    auto putOutcome = bsClient.putObject(
            models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value()) << putOutcome.error().getMessage();

    auto getOutcome = bsClient.getObject(models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    ASSERT_TRUE(getOutcome.has_value()) << getOutcome.error().getMessage();
    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());

    (void)bsClient.deleteObject(models::DeleteObjectRequest().setBucket(bucketName_).setKey(key));
}

// Drive the same space through a plain OSSClient using a helper-built full name.
TEST_F(AgenticBucketSpaceTest, SpaceHelper) {
    agentic::BucketSpaceHelper helper(agentictest::testConfig());
    auto fullName = helper.toBucketName(bucketName_);
    EXPECT_EQ(agentictest::buildFullName(bucketName_, "bs-apsr"), fullName);

    OSSClient plainClient(agentictest::testConfig());

    auto getInfoOutcome = plainClient.getBucketInfo(models::GetBucketInfoRequest().setBucket(fullName));
    ASSERT_TRUE(getInfoOutcome.has_value()) << getInfoOutcome.error().getMessage();

    auto key = "cpp-sdk-test-object-" + agentictest::randStr(6);
    const std::string content = "hello helper";

    auto putOutcome = plainClient.putObject(
            models::PutObjectRequest().setBucket(fullName).setKey(key).setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value()) << putOutcome.error().getMessage();

    auto getOutcome = plainClient.getObject(models::GetObjectRequest().setBucket(fullName).setKey(key));
    ASSERT_TRUE(getOutcome.has_value()) << getOutcome.error().getMessage();
    std::ostringstream ss;
    ss << getOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());

    (void)plainClient.deleteObject(models::DeleteObjectRequest().setBucket(fullName).setKey(key));
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
