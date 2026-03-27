#include <gtest/gtest.h>

#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/utils/Outcome.h"


namespace alibabacloud {

namespace oss2 {

class StubResult : public ResultModel {
    std::string strResult;
};

using StubResultOutcome = Outcome<StubResult, OperationError>;

TEST(OutcomeTest, BuildOutcome2) {
    auto outcome = StubResultOutcome(StubResult{});
    EXPECT_TRUE(outcome.isSuccess());

    outcome = StubResultOutcome(OperationError{});
    EXPECT_FALSE(outcome.isSuccess());
}

} // namespace oss2
} // namespace alibabacloud
