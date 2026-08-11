#include <gtest/gtest.h>

#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

// Error propagation with invalid credentials. Create is rejected with 403; Get and
// List are relaxed because the service answers Get with 404 under an invalid AK.
TEST(AgenticBucketServerErrorsTest, InvalidCredentials) {
    auto client = agentictest::makeAgenticClient(false);
    auto bucket = agentictest::genBucketName();

    auto createOutcome =
            client->createAgenticBucket(agentic::models::CreateAgenticBucketRequest().setBucket(bucket));
    ASSERT_FALSE(createOutcome.has_value());
    EXPECT_EQ(403, createOutcome.error().getStatusCode());
    EXPECT_FALSE(createOutcome.error().getRequestId().empty());

    auto getOutcome = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucket));
    ASSERT_FALSE(getOutcome.has_value());
    EXPECT_NE(0, getOutcome.error().getStatusCode());

    auto listOutcome = client->listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_FALSE(listOutcome.has_value());
    EXPECT_NE(0, listOutcome.error().getStatusCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
