
#include "alibabacloud/oss2/Error.h"

namespace alibabacloud {
namespace oss2 {

namespace {

class credentials_error_category : public std::error_category {
  public:
    const char* name() const noexcept override { return "oss2.credentials"; }

    std::string message(int ev) const override {
        switch (static_cast<CredentialsErrorCode>(ev)) {
            case CredentialsErrorCode::Empty:
                return "credentials are empty";
            case CredentialsErrorCode::FetchError:
                return "failed to fetch credentials";
            case CredentialsErrorCode::ProviderNull:
                return "credentials provider is null";
            default:
                return "unknown credentials error";
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        auto ec = static_cast<CredentialsErrorCode>(code);
        if (cond == make_error_condition(ErrorCondition::Retryable)) {
            return ec == CredentialsErrorCode::FetchError;
        }
        if (cond == make_error_condition(ErrorCondition::AuthenticationError)) {
            return ec == CredentialsErrorCode::Empty
                || ec == CredentialsErrorCode::ProviderNull;
        }
        return false;
    }
};

} // namespace

std::error_code make_error_code(CredentialsErrorCode e) {
    static const credentials_error_category cat{};
    return {static_cast<int>(e), cat};
}

} // namespace oss2
} // namespace alibabacloud
