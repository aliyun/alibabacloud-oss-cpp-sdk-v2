
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class serde_error_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.serde"; }

    std::string message(int ev) const override {
        switch (static_cast<SerdeErrorCode>(ev)) {
            case SerdeErrorCode::DeserializationFailed:
                return "deserialization failed";
            default:
                return "unknown serde error";
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        (void)code;
        if (cond == make_error_condition(ErrorCondition::NonRetryable)) {
            return static_cast<SerdeErrorCode>(code) == SerdeErrorCode::DeserializationFailed;
        }
        return false;
    }
};

} // namespace

std::error_code make_error_code(SerdeErrorCode e) {
    static const serde_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
