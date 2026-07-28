#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "src/agentic/AgenticUtils.h"
#include "src/agentic/SerdeAgenticBucket.h"
#include "src/client/sync/OSSClientUtils.h"

namespace alibabacloud {
namespace oss2 {
namespace agentic {

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
