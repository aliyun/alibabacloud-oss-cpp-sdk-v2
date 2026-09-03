#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/agentic/AgenticFwd.h"

namespace alibabacloud {
namespace oss2 {

struct ClientConfiguration;

namespace agentic {

/**
 * @brief Client for managing agentic buckets.
 *
 * The agentic bucket client resolves a logical bucket prefix into the physical
 * bucket name {prefix}-{accountId}-{region}-ab-apsr for signing and host
 * construction. The account id must be set in ClientConfiguration.
 */
class ALIBABACLOUD_OSS_API OSSAgenticBucketClient final {
  public:
    explicit OSSAgenticBucketClient(const ClientConfiguration& config);

    /**
     * @brief Creates an agentic bucket.
     */
    CreateAgenticBucketOutcome createAgenticBucket(const models::CreateAgenticBucketRequest& request,
                                                   const OperationOptions* options = nullptr);

    /**
     * @brief Deletes an agentic bucket.
     */
    DeleteAgenticBucketOutcome deleteAgenticBucket(const models::DeleteAgenticBucketRequest& request,
                                                   const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about an agentic bucket.
     */
    GetAgenticBucketOutcome getAgenticBucket(const models::GetAgenticBucketRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief Lists the agentic buckets owned by the requester.
     */
    ListAgenticBucketsOutcome listAgenticBuckets(const models::ListAgenticBucketsRequest& request,
                                                 const OperationOptions* options = nullptr);

    /**
     * @brief Updates the status of an agentic bucket.
     */
    PutAgenticBucketStatusOutcome putAgenticBucketStatus(const models::PutAgenticBucketStatusRequest& request,
                                                         const OperationOptions* options = nullptr);

    /**
     * @brief Lists the bucket spaces of an agentic bucket.
     */
    ListBucketSpacesOutcome listBucketSpaces(const models::ListBucketSpacesRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief Invokes a raw operation through the agentic bucket resolver and signer.
     */
    OperationResult invokeOperation(const OperationInput& input, const OperationOptions* options = nullptr);

    template <typename RequestT>
    struct OperationTraits;

  private:
    OSSClient client_;
};

// OperationTraits specializations (used by the paginator)
template <>
struct OSSAgenticBucketClient::OperationTraits<models::ListAgenticBucketsRequest> {
    using OutcomeType = ListAgenticBucketsOutcome;
    static constexpr auto method = &OSSAgenticBucketClient::listAgenticBuckets;
};

template <>
struct OSSAgenticBucketClient::OperationTraits<models::ListBucketSpacesRequest> {
    using OutcomeType = ListBucketSpacesOutcome;
    static constexpr auto method = &OSSAgenticBucketClient::listBucketSpaces;
};

/**
 * @brief Creates an OSSClient bound to an agentic bucket space.
 *
 * The returned client resolves a logical bucket space prefix into the physical
 * bucket name {prefix}-{accountId}-{region}-bs-apsr for signing and host
 * construction, then exposes the full object operation surface of OSSClient.
 * The account id must be set in ClientConfiguration.
 */
ALIBABACLOUD_OSS_API OSSClient makeBucketSpaceClient(const ClientConfiguration& config);

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
