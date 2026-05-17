
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class signer_error_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.signer"; }

    std::string message(int ev) const override {
        switch (static_cast<SignerErrorCode>(ev)) {
            case SignerErrorCode::SignFailed:
                return "signing failed";
            default:
                return "unknown signer error";
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        (void)code;
        if (cond == make_error_condition(ErrorCondition::AuthenticationError)) {
            return static_cast<SignerErrorCode>(code) == SignerErrorCode::SignFailed;
        }
        return false;
    }
};

} // namespace

std::error_code make_error_code(SignerErrorCode e) {
    static const signer_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
