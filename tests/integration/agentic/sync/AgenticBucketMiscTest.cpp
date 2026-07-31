#include <gtest/gtest.h>

#include "Config.h"
#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

// Miscellaneous agentic integration scenarios that do not belong to the Basic,
// Lifecycle, or Space suites. Add future one-offs here.

// Path-style addressing across both the agentic bucket client and the bucket
// space client. Path-style may be disabled on the endpoint; the probe detects
// SecondLevelDomainForbidden and skips rather than fails, since that is an
// endpoint capability and not an SDK defect.
TEST(AgenticBucketMiscTest, PathStyle) {
    auto bucket = agentictest::genBucketName();

    // Create the bucket with the default (virtual-hosted) client so the fixture
    // stands regardless of whether path-style turns out to be allowed.
    auto client = agentictest::makeAgenticClient();
    auto createOutcome =
            client->createAgenticBucket(agentic::models::CreateAgenticBucketRequest().setBucket(bucket));
    ASSERT_TRUE(createOutcome.has_value()) << createOutcome.error().getMessage();

    // Disable + reap the bucket on the way out, whatever happens below.
    struct Reaper {
        std::string bucket;
        ~Reaper() { agentictest::disableAndReap(bucket); }
    } reaper{bucket};

    auto psClient = agentictest::makeAgenticClientPathStyle();

    // Probe: a path-style GET carrying the bucket. ListAgenticBuckets is
    // service-level (no bucket label) so its URL is identical in both styles and
    // cannot probe path-style; GetAgenticBucket carries the bucket and does.
    auto getOutcome = psClient->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucket));
    if (!getOutcome.has_value() && getOutcome.error().getCode() == "SecondLevelDomainForbidden") {
        GTEST_SKIP() << "path-style addressing is not allowed on this endpoint (SecondLevelDomainForbidden)";
    }
    ASSERT_TRUE(getOutcome.has_value()) << getOutcome.error().getMessage();
    EXPECT_EQ(200, getOutcome.value().getStatusCode());
    EXPECT_TRUE(getOutcome.value().hasAgenticBucketInfo());

    // Agentic bucket client over path-style.
    auto listOutcome = psClient->listBucketSpaces(agentic::models::ListBucketSpacesRequest().setBucket(bucket));
    ASSERT_TRUE(listOutcome.has_value()) << listOutcome.error().getMessage();
    EXPECT_EQ(200, listOutcome.value().getStatusCode());

    // Create one bucket space (via the default client) shared by the checks below.
    auto bsClient = agentictest::makeBsClient();
    auto putBucketOutcome = bsClient.putBucket(
            models::PutBucketRequest().setBucket(bucket).setAgenticBucket(
                    agentictest::buildFullName(bucket, "ab-apsr")));
    ASSERT_TRUE(putBucketOutcome.has_value()) << putBucketOutcome.error().getMessage();

    // Bucket space client over path-style.
    auto psBsClient = agentictest::makeBsClientPathStyle();

    auto key = "cpp-sdk-test-object-" + agentictest::randStr(6);
    const std::string content = "hello path-style";

    auto putObjOutcome = psBsClient.putObject(
            models::PutObjectRequest().setBucket(bucket).setKey(key).setBody(RequestBody::fromString(content)));
    // Path-style may be forbidden on the bucket space endpoint independently of the
    // agentic bucket endpoint (different domain), so guard the first bucket-space
    // path-style call separately. A pass here implies GetBucketAcl below is fine too.
    if (!putObjOutcome.has_value() && putObjOutcome.error().getCode() == "SecondLevelDomainForbidden") {
        (void)bsClient.deleteBucket(models::DeleteBucketRequest().setBucket(bucket));
        GTEST_SKIP() << "path-style addressing is not allowed for the bucket space endpoint (SecondLevelDomainForbidden)";
    }
    ASSERT_TRUE(putObjOutcome.has_value()) << putObjOutcome.error().getMessage();

    auto getObjOutcome = psBsClient.getObject(models::GetObjectRequest().setBucket(bucket).setKey(key));
    ASSERT_TRUE(getObjOutcome.has_value()) << getObjOutcome.error().getMessage();
    std::ostringstream ss;
    ss << getObjOutcome.value().getBody()->rdbuf();
    EXPECT_EQ(content, ss.str());

    (void)psBsClient.deleteObject(models::DeleteObjectRequest().setBucket(bucket).setKey(key));

    auto getAclOutcome = psBsClient.getBucketAcl(models::GetBucketAclRequest().setBucket(bucket));
    ASSERT_TRUE(getAclOutcome.has_value()) << getAclOutcome.error().getMessage();
    EXPECT_EQ(200, getAclOutcome.value().getStatusCode());

    // Clean up the bucket space; the reaper handles the agentic bucket.
    (void)bsClient.deleteBucket(models::DeleteBucketRequest().setBucket(bucket));
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
