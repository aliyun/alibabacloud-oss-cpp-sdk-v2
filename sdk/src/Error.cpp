

#include "alibabacloud/oss2/Error.h"


namespace alibabacloud {
namespace oss2 {

namespace detail {
class error_codes : public std::error_category {
    const char* name() const noexcept override {
        return "SdkError";
    }

    std::string message(int ev) const override {
        switch (static_cast<SdkErrorCode>(ev)) {
            case SdkErrorCode::NO_ERROR:
                return "no error";
            default:
                return "unknown sdk error";
        }
    }
};
} // namespace detail


std::error_code make_error_code(SdkErrorCode e) {
    static detail::error_codes const cat{};
    return std::error_code{static_cast<std::underlying_type<SdkErrorCode>::type>(e), cat};
}


} // namespace oss2
} // namespace alibabacloud
