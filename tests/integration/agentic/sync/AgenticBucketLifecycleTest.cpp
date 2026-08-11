#include <gtest/gtest.h>

#include "Config.h"
#include "agentic/AgenticTestHelpers.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

// Connectivity only, on its own bucket so that disabling does not break the other
// scenarios: PutStatus(Disabled) succeeds, then Delete is not yet ready (409 /
// AgenticBucketNotReady). The two-phase delete needs ~24h and cannot be asserted
// end-to-end in CI.
class AgenticBucketLifecycleTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        bucketName_ = agentictest::genBucketName();
        auto client = agentictest::makeAgenticClient();
        auto outcome =
                client->createAgenticBucket(agentic::models::CreateAgenticBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value()) << (outcome.has_value() ? "" : outcome.error().getMessage());
    }

    static void TearDownTestCase() {
        // The bucket is already Disabled by the test; just run the reaper.
        agentictest::reapDisabled();
    }

  public:
    static std::string bucketName_;
};

std::string AgenticBucketLifecycleTest::bucketName_ = "";

TEST_F(AgenticBucketLifecycleTest, DisableThenDelete) {
    auto client = agentictest::makeAgenticClient();

    auto putOutcome = client->putAgenticBucketStatus(
            agentic::models::PutAgenticBucketStatusRequest().setBucket(bucketName_).setAgenticBucketStatus(
                    agentic::models::AgenticBucketStatus().setStatus("Disabled")));
    ASSERT_TRUE(putOutcome.has_value()) << putOutcome.error().getMessage();
    EXPECT_EQ(200, putOutcome.value().getStatusCode());

    auto delOutcome =
            client->deleteAgenticBucket(agentic::models::DeleteAgenticBucketRequest().setBucket(bucketName_));
    ASSERT_FALSE(delOutcome.has_value());
    EXPECT_TRUE(delOutcome.error().getStatusCode() == 409 ||
                delOutcome.error().getCode() == "AgenticBucketNotReady")
            << "expected AgenticBucketNotReady/409, got code=" << delOutcome.error().getCode()
            << " status=" << delOutcome.error().getStatusCode();
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
