#pragma once

#include "alibabacloud/oss2/Types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {

// forward declare
class Signer;
class CredentialsProvider;
class Retryer;
class HttpTransport;
class AsyncHttpTransport;
struct OperationInput;

/**
 * @brief Resolved, immutable options used internally by ClientImpl.
 *
 * ClientOptions is the internal counterpart of ClientConfiguration.
 * During client construction, ClientImpl::resolveConfig() converts a
 * user-supplied ClientConfiguration into a fully resolved ClientOptions
 * where every field has a concrete value (no std::optional).
 *
 * Users do not construct this struct directly; it is created by the SDK.
 *
 * @see ClientConfiguration  The user-facing configuration struct.
 */
struct ALIBABACLOUD_OSS_API ClientOptions {
    /// The product name, defaults to "oss".
    std::string product;

    /// The region in which the bucket is located.
    /// Resolved from ClientConfiguration::region.
    std::string region;

    /// The account id. Resolved from ClientConfiguration::accountId.
    std::string accountId;

    /// The resolved endpoint URL for API calls.
    /// Derived from ClientConfiguration::endpoint or constructed from @c region.
    std::string endpoint;

    /// The credentials provider to use when signing requests.
    std::shared_ptr<CredentialsProvider> credentialsProvider;

    /// The signer instance used to sign each request.
    /// Resolved from ClientConfiguration::signer or created based on
    /// ClientConfiguration::signatureVersion.
    std::shared_ptr<Signer> signer;

    /// Guides how HTTP requests should be retried in case of recoverable failures.
    /// Resolved from ClientConfiguration::retryer or created with
    /// ClientConfiguration::retryMaxAttempts.
    std::shared_ptr<Retryer> retryer;

    /// The HTTP client used to invoke API calls.
    /// Resolved from ClientConfiguration::httpTransport or defaults to CurlHttpClient.
    std::shared_ptr<HttpTransport> httpTransport;

    /// The async HTTP client used by OSSAsyncClient.
    /// Resolved from ClientConfiguration::asyncHttpTransport or defaults to CurlMultiTransport.
    std::shared_ptr<AsyncHttpTransport> asyncHttpTransport;

    /// The addressing style for bucket endpoints.
    /// Resolved from the usePathStyle / useCName configuration flags.
    /// - VirtualHosted: @c https://bucket.oss-cn-hangzhou.aliyuncs.com/key (default)
    /// - Path:          @c https://oss-cn-hangzhou.aliyuncs.com/bucket/key
    /// - CName:         uses the custom domain as-is
    AddressStyleType addressStyle{};

    /// Bitmask of enabled SDK features.
    /// @see FeatureFlagsType for individual flag definitions (e.g., CorrectClockSkew,
    ///      AutoDetectMimeType, EnableCRC64CheckUpload).
    int featureFlags{};

    /// Additional signable headers to include in the request signature.
    std::vector<std::string> additionalHeaders;

    /// Resolves the physical bucket name from an operation input. Used by the
    /// agentic bucket client to expand the logical bucket prefix for signing.
    /// Returns an empty string when no resolution applies.
    std::function<std::string(const OperationInput&)> bucketNameResolver;

    /// Builds the full request URL (scheme://host/path) from an operation input.
    /// When set, it fully replaces the default host/path construction. Used by the
    /// agentic bucket client to route requests to the physical virtual-hosted host.
    std::function<std::string(const OperationInput&)> endpointProvider;
};

/// Functional option for customizing ClientOptions.
using ClientOptionsFn = std::function<void(ClientOptions&)>;

/// A list of functional options applied during client construction.
using ClientOptionsFns = std::vector<ClientOptionsFn>;

} // namespace oss2
} // namespace alibabacloud