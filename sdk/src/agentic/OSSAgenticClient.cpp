#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/internal/Url.h"
#include "src/agentic/SerdeAgenticBucket.h"
#include "src/utils/Utils.h"
#include "src/client/sync/OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {
namespace agentic {

static ClientOptionsFns makeAgenticOptionsFns(const std::string& accountId, const std::string& region,
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
        opts.endpointProvider = [buildName, scheme, authority, addressStyle](const OperationInput& input) -> std::string {
            std::vector<std::string> paths;
            paths.reserve(2);
            auto host = authority;

            if (input.bucket.has_value()) {
                switch (addressStyle) {
                    case AddressStyleType::Path:
                        paths.emplace_back(buildName(input));
                        if (!input.key.has_value()) {
                            paths.emplace_back("");
                        }
                        break;
                    default: host = buildName(input) + "." + authority; break;
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

static OSSClient makeScopedClient(const ClientConfiguration& config, const std::string& suffix) {
    auto cfg = config;
    std::string ua = "agentic-client";
    if (cfg.userAgent.has_value() && !cfg.userAgent.value().empty()) {
        ua += "/" + cfg.userAgent.value();
    }
    cfg.userAgent = ua;
    auto fns = makeAgenticOptionsFns(cfg.accountId.value_or(""), cfg.region.value_or(""), suffix);
    return OSSClient(cfg, fns);
}

OSSAgenticBucketClient::OSSAgenticBucketClient(const ClientConfiguration& config)
    : client_(makeScopedClient(config, "-ab-apsr")) {}

CreateAgenticBucketOutcome OSSAgenticBucketClient::createAgenticBucket(
    const models::CreateAgenticBucketRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    auto input = transform::fromCreateAgenticBucket(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toCreateAgenticBucket(std::move(std::get<OperationOutput>(result)));
}

DeleteAgenticBucketOutcome OSSAgenticBucketClient::deleteAgenticBucket(
    const models::DeleteAgenticBucketRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    auto input = transform::fromDeleteAgenticBucket(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toDeleteAgenticBucket(std::move(std::get<OperationOutput>(result)));
}

GetAgenticBucketOutcome OSSAgenticBucketClient::getAgenticBucket(const models::GetAgenticBucketRequest& request,
                                                                const OperationOptions* options) {
    requiredField(Bucket);
    auto input = transform::fromGetAgenticBucket(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetAgenticBucket(std::move(std::get<OperationOutput>(result)));
}

ListAgenticBucketsOutcome OSSAgenticBucketClient::listAgenticBuckets(
    const models::ListAgenticBucketsRequest& request, const OperationOptions* options) {
    auto input = transform::fromListAgenticBuckets(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toListAgenticBuckets(std::move(std::get<OperationOutput>(result)));
}

PutAgenticBucketStatusOutcome OSSAgenticBucketClient::putAgenticBucketStatus(
    const models::PutAgenticBucketStatusRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredHasField(AgenticBucketStatus);
    auto input = transform::fromPutAgenticBucketStatus(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutAgenticBucketStatus(std::move(std::get<OperationOutput>(result)));
}

ListBucketSpacesOutcome OSSAgenticBucketClient::listBucketSpaces(const models::ListBucketSpacesRequest& request,
                                                               const OperationOptions* options) {
    requiredField(Bucket);
    auto input = transform::fromListBucketSpaces(request);
    auto result = client_.invokeOperation(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toListBucketSpaces(std::move(std::get<OperationOutput>(result)));
}

OperationResult OSSAgenticBucketClient::invokeOperation(const OperationInput& input, const OperationOptions* options) {
    return client_.invokeOperation(input, options);
}

OSSClient makeBucketSpaceClient(const ClientConfiguration& config) {
    return makeScopedClient(config, "-bs-apsr");
}

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
