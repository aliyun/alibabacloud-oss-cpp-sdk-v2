
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class error_condition_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.condition"; }

    std::string message(int ev) const override {
        switch (static_cast<ErrorCondition>(ev)) {
            case ErrorCondition::Retryable:
                return "retryable error";
            case ErrorCondition::NonRetryable:
                return "non-retryable error";
            case ErrorCondition::Canceled:
                return "operation canceled";
            case ErrorCondition::InvalidArgument:
                return "invalid argument";
            case ErrorCondition::AuthenticationError:
                return "authentication error";
            default:
                return "unknown condition";
        }
    }
};

} // namespace

std::error_condition make_error_condition(ErrorCondition e) {
    static const error_condition_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
