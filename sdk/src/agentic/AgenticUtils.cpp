#include "src/agentic/AgenticUtils.h"

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "src/internal/Url.h"
#include "src/utils/Utils.h"

#include <cstddef>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace agentic {

namespace {
// Maximum length of a single DNS label; the physical bucket name occupies the
// leftmost label of the virtual-hosted host and must not exceed it.
constexpr std::size_t kMaxHostLabelLength = 63;
} // namespace

ClientOptionsFns makeAgenticOptionsFns(const std::string& accountId, const std::string& region,
                                       const std::string& suffix) {
    ClientOptionsFns fns;
    fns.emplace_back([accountId, region, suffix](ClientOptions& opts) {
        auto buildName = [accountId, region, suffix](const OperationInput& input) -> std::string {
            if (!input.bucket.has_value()) {
                return {};
            }
            return input.bucket.value() + "-" + accountId + "-" + region + suffix;
        };
        opts.bucketNameResolver = buildName;

        auto url = internal::Url(opts.endpoint);
        auto scheme = url.scheme();
        auto authority = url.authority();
        auto addressStyle = opts.addressStyle;
        bool accountIdEmpty = accountId.empty();
        bool regionEmpty = region.empty();
        opts.endpointProvider = [buildName, scheme, authority, addressStyle, accountIdEmpty, regionEmpty](
                                    const OperationInput& input, std::error_code& ec) -> std::string {
            std::vector<std::string> paths;
            paths.reserve(2);
            auto host = authority;

            if (input.bucket.has_value()) {
                if (accountIdEmpty) {
                    ec = make_error_code(ClientErrorCode::AccountIdNull);
                    return {};
                }
                if (regionEmpty) {
                    ec = make_error_code(ClientErrorCode::EndpointRegionNull);
                    return {};
                }
                auto name = buildName(input);
                switch (addressStyle) {
                    case AddressStyleType::Path:
                        paths.emplace_back(name);
                        if (!input.key.has_value()) {
                            paths.emplace_back("");
                        }
                        break;
                    default:
                        if (name.size() > kMaxHostLabelLength) {
                            ec = make_error_code(ClientErrorCode::HostLabelTooLong);
                            return {};
                        }
                        host = name + "." + authority;
                        break;
                }
            }

            if (input.key.has_value()) {
                paths.emplace_back(utils::UrlEncodePath(input.key.value()));
            }

            return scheme + "://" + host + "/" + utils::StringJoin(paths, "/");
        };
    });
    return fns;
}

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
