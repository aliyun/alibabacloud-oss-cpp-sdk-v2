#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/agentic/AgenticFwd.h"

namespace alibabacloud {
namespace oss2 {

struct ClientConfiguration;

namespace agentic {

/**
 * @brief Asynchronous client for managing agentic buckets.
 *
 * The agentic bucket client resolves a logical bucket prefix into the physical
 * bucket name {prefix}-{accountId}-{region}-ab-apsr for signing and host
 * construction. The account id must be set in ClientConfiguration.
 */
class ALIBABACLOUD_OSS_API OSSAsyncAgenticBucketClient final {
  public:
    explicit OSSAsyncAgenticBucketClient(const ClientConfiguration& config);

    void createAgenticBucketAsync(const models::CreateAgenticBucketRequest& request,
                                  const CreateAgenticBucketAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    void deleteAgenticBucketAsync(const models::DeleteAgenticBucketRequest& request,
                                  const DeleteAgenticBucketAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    void getAgenticBucketAsync(const models::GetAgenticBucketRequest& request,
                               const GetAgenticBucketAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    void listAgenticBucketsAsync(const models::ListAgenticBucketsRequest& request,
                                 const ListAgenticBucketsAsyncCallback& callback,
                                 const OperationOptions* options = nullptr);

    void putAgenticBucketStatusAsync(const models::PutAgenticBucketStatusRequest& request,
                                     const PutAgenticBucketStatusAsyncCallback& callback,
                                     const OperationOptions* options = nullptr);

    void listBucketSpacesAsync(const models::ListBucketSpacesRequest& request,
                               const ListBucketSpacesAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    /**
     * @brief Asynchronously invokes a raw operation through the agentic bucket resolver and signer.
     */
    void invokeOperationAsync(const OperationInput& input, const OperationCallback& callback,
                              const OperationOptions* options = nullptr);

    template <typename RequestT>
    struct OperationTraits;

    /**
     * @brief Asynchronously invokes an operation and returns a std::future.
     *
     * The outcome type is automatically deduced from the request type via OperationTraits.
     */
    template <typename RequestT>
    std::future<typename OperationTraits<RequestT>::OutcomeType> asyncCall(const RequestT& request,
                                                                           const OperationOptions* options = nullptr) {
        using Traits = OperationTraits<RequestT>;
        auto promise = std::make_shared<std::promise<typename Traits::OutcomeType>>();
        (this->*Traits::method)(request, typename Traits::CallbackType([promise](typename Traits::OutcomeType result) {
                                    promise->set_value(std::move(result));
                                }),
                                options);
        return promise->get_future();
    }

  private:
    OSSAsyncClient client_;
};

// OperationTraits specializations (used by asyncCall)
template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::CreateAgenticBucketRequest> {
    using OutcomeType = CreateAgenticBucketOutcome;
    using CallbackType = CreateAgenticBucketAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::createAgenticBucketAsync;
};

template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::DeleteAgenticBucketRequest> {
    using OutcomeType = DeleteAgenticBucketOutcome;
    using CallbackType = DeleteAgenticBucketAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::deleteAgenticBucketAsync;
};

template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::GetAgenticBucketRequest> {
    using OutcomeType = GetAgenticBucketOutcome;
    using CallbackType = GetAgenticBucketAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::getAgenticBucketAsync;
};

template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::ListAgenticBucketsRequest> {
    using OutcomeType = ListAgenticBucketsOutcome;
    using CallbackType = ListAgenticBucketsAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::listAgenticBucketsAsync;
};

template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::PutAgenticBucketStatusRequest> {
    using OutcomeType = PutAgenticBucketStatusOutcome;
    using CallbackType = PutAgenticBucketStatusAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::putAgenticBucketStatusAsync;
};

template <>
struct OSSAsyncAgenticBucketClient::OperationTraits<models::ListBucketSpacesRequest> {
    using OutcomeType = ListBucketSpacesOutcome;
    using CallbackType = ListBucketSpacesAsyncCallback;
    static constexpr auto method = &OSSAsyncAgenticBucketClient::listBucketSpacesAsync;
};

/**
 * @brief Creates an OSSAsyncClient bound to an agentic bucket space.
 *
 * The returned client resolves a logical bucket space prefix into the physical
 * bucket name {prefix}-{accountId}-{region}-bs-apsr for signing and host
 * construction, then exposes the full object operation surface of OSSAsyncClient.
 * The account id must be set in ClientConfiguration.
 */
ALIBABACLOUD_OSS_API OSSAsyncClient makeAsyncBucketSpaceClient(const ClientConfiguration& config);

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
