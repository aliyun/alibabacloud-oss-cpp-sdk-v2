#include <gtest/gtest.h>

#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

// Error propagation with invalid credentials. Create and List are rejected with 403
// InvalidAccessKeyId, while Get answers 404 NoSuchAgenticBucket: the service resolves
// bucket existence before it validates the credentials. The ec values are only checked
// for presence, they are server-internal diagnostics and not part of the contract.
TEST(AgenticBucketServerErrorsTest, InvalidCredentials) {
    auto client = agentictest::makeAgenticClient(false);
    auto bucket = agentictest::genBucketName();

    auto createOutcome =
            client->createAgenticBucket(agentic::models::CreateAgenticBucketRequest().setBucket(bucket));
    ASSERT_FALSE(createOutcome.has_value());
    EXPECT_EQ(403, createOutcome.error().getStatusCode());
    EXPECT_EQ("InvalidAccessKeyId", createOutcome.error().getCode());
    EXPECT_FALSE(createOutcome.error().getEC().empty());
    EXPECT_FALSE(createOutcome.error().getRequestId().empty());

    auto getOutcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucket));
    ASSERT_FALSE(getOutcome.has_value());
    EXPECT_EQ(404, getOutcome.error().getStatusCode());
    EXPECT_EQ("NoSuchAgenticBucket", getOutcome.error().getCode());
    EXPECT_FALSE(getOutcome.error().getEC().empty());
    EXPECT_FALSE(getOutcome.error().getRequestId().empty());

    auto listOutcome = client->listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_FALSE(listOutcome.has_value());
    EXPECT_EQ(403, listOutcome.error().getStatusCode());
    EXPECT_EQ("InvalidAccessKeyId", listOutcome.error().getCode());
    EXPECT_FALSE(listOutcome.error().getEC().empty());
    EXPECT_FALSE(listOutcome.error().getRequestId().empty());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
