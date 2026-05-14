#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/utils/Outcome.h"


namespace alibabacloud {

namespace oss2 {

class StubResult : public ResultModel {
  public:
    std::string val;
};

using StubResultOutcome = Outcome<StubResult, OperationError>;

TEST(OutcomeTest, HasValue) {
    auto success = StubResultOutcome(StubResult{});
    EXPECT_TRUE(success.has_value());
    EXPECT_TRUE(static_cast<bool>(success));

    auto failure = StubResultOutcome(makeUnexpected(OperationError{}));
    EXPECT_FALSE(failure.has_value());
    EXPECT_FALSE(static_cast<bool>(failure));
}

TEST(OutcomeTest, Value) {
    StubResult r;
    r.val = "hello";
    auto outcome = StubResultOutcome(std::move(r));
    EXPECT_EQ(outcome.value().val, "hello");

    const auto& cref = outcome;
    EXPECT_EQ(cref.value().val, "hello");
}

TEST(OutcomeTest, Error) {
    auto outcome = StubResultOutcome(makeUnexpected(OperationError(SdkErrorCode::ARGUMENT_INVALID,
        {{"Code", "TestError"}, {"Message", "test message"}})));
    EXPECT_EQ(outcome.error().getCode(), "TestError");

    const auto& cref = outcome;
    EXPECT_EQ(cref.error().getCode(), "TestError");
}

TEST(OutcomeTest, DerefOperators) {
    StubResult r;
    r.val = "deref";
    auto outcome = StubResultOutcome(std::move(r));
    EXPECT_EQ((*outcome).val, "deref");
    EXPECT_EQ(outcome->val, "deref");
}

TEST(OutcomeTest, ValueOr) {
    auto success = StubResultOutcome(StubResult{});
    success.value().val = "real";

    StubResult fallback;
    fallback.val = "fallback";

    EXPECT_EQ(success.value_or(std::move(fallback)).val, "real");

    auto failure = StubResultOutcome(makeUnexpected(OperationError{}));
    StubResult fallback2;
    fallback2.val = "fallback2";
    EXPECT_EQ(failure.value_or(std::move(fallback2)).val, "fallback2");
}

#ifndef ALIBABACLOUD_OSS_USE_STD_EXPECTED
TEST(OutcomeTest, LegacyInterface) {
    auto success = StubResultOutcome(StubResult{});
    EXPECT_TRUE(success.isSuccess());
    success.getResult().val = "legacy";
    EXPECT_EQ(success.getResult().val, "legacy");

    auto failure = StubResultOutcome(makeUnexpected(OperationError(SdkErrorCode::ARGUMENT_INVALID,
        {{"Code", "Err"}})));
    EXPECT_FALSE(failure.isSuccess());
    EXPECT_EQ(failure.getError().getCode(), "Err");
}
#endif

TEST(OutcomeTest, MoveConstruction) {
    StubResult r;
    r.val = "moved";
    auto outcome = StubResultOutcome(std::move(r));
    auto moved = std::move(outcome);
    EXPECT_TRUE(moved.has_value());
    EXPECT_EQ(moved.value().val, "moved");
}

} // namespace oss2
} // namespace alibabacloud
