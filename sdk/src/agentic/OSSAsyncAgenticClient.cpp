#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/internal/Url.h"
#include "src/agentic/SerdeAgenticBucket.h"
#include "src/utils/Utils.h"
#include "src/client/async/OSSAsyncClientUtils.h"

namespace alibabacloud {
namespace oss2 {
namespace agentic {

static ClientOptionsFns makeAsyncAgenticOptionsFns(const std::string& accountId, const std::string& region,
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

static OSSAsyncClient makeScopedAsyncClient(const ClientConfiguration& config, const std::string& suffix) {
    auto cfg = config;
    std::string ua = "agentic-client";
    if (cfg.userAgent.has_value() && !cfg.userAgent.value().empty()) {
        ua += "/" + cfg.userAgent.value();
    }
    cfg.userAgent = ua;
    auto fns = makeAsyncAgenticOptionsFns(cfg.accountId.value_or(""), cfg.region.value_or(""), suffix);
    return OSSAsyncClient(cfg, fns);
}

OSSAsyncAgenticBucketClient::OSSAsyncAgenticBucketClient(const ClientConfiguration& config)
    : client_(makeScopedAsyncClient(config, "-ab-apsr")) {}

void OSSAsyncAgenticBucketClient::createAgenticBucketAsync(const models::CreateAgenticBucketRequest& request,
                                                           const CreateAgenticBucketAsyncCallback& callback,
                                                           const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    auto input = transform::fromCreateAgenticBucket(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toCreateAgenticBucket(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::deleteAgenticBucketAsync(const models::DeleteAgenticBucketRequest& request,
                                                           const DeleteAgenticBucketAsyncCallback& callback,
                                                           const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    auto input = transform::fromDeleteAgenticBucket(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toDeleteAgenticBucket(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::getAgenticBucketAsync(const models::GetAgenticBucketRequest& request,
                                                        const GetAgenticBucketAsyncCallback& callback,
                                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    auto input = transform::fromGetAgenticBucket(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toGetAgenticBucket(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::listAgenticBucketsAsync(const models::ListAgenticBucketsRequest& request,
                                                          const ListAgenticBucketsAsyncCallback& callback,
                                                          const OperationOptions* options) {
    auto input = transform::fromListAgenticBuckets(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toListAgenticBuckets(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::putAgenticBucketStatusAsync(const models::PutAgenticBucketStatusRequest& request,
                                                              const PutAgenticBucketStatusAsyncCallback& callback,
                                                              const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    requiredHasFieldAsync(AgenticBucketStatus);
    auto input = transform::fromPutAgenticBucketStatus(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toPutAgenticBucketStatus(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::listBucketSpacesAsync(const models::ListBucketSpacesRequest& request,
                                                        const ListBucketSpacesAsyncCallback& callback,
                                                        const OperationOptions* options) {
    requiredFieldAsync(Bucket);
    auto input = transform::fromListBucketSpaces(request);
    client_.invokeOperationAsync(
        input,
        [callback](OperationResult result) {
            if (std::holds_alternative<OperationError>(result)) {
                callback(makeUnexpected(std::get<OperationError>(std::move(result))));
                return;
            }
            callback(transform::toListBucketSpaces(std::move(std::get<OperationOutput>(result))));
        },
        options);
}

void OSSAsyncAgenticBucketClient::invokeOperationAsync(const OperationInput& input, const OperationCallback& callback,
                                                       const OperationOptions* options) {
    client_.invokeOperationAsync(input, callback, options);
}

OSSAsyncClient makeAsyncBucketSpaceClient(const ClientConfiguration& config) {
    return makeScopedAsyncClient(config, "-bs-apsr");
}

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
