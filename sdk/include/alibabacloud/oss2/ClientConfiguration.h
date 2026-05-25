#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <memory>
#include <optional>
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
class Executor;
class ScheduledExecutor;

/**
 * @brief User-facing configuration for constructing an OSSClient.
 *
 * All fields are optional. Unset fields are resolved to sensible defaults
 * inside ClientImpl during client construction (e.g., region is used to
 * derive the endpoint, a StandardRetryer is created when retryer is null).
 *
 * @code
 *   ClientConfiguration cfg;
 *   cfg.region       = "cn-hangzhou";
 *   cfg.credentialsProvider = std::make_shared<StaticCredentialsProvider>(ak, sk);
 *   cfg.connectTimeout      = 10000;   // 10 seconds
 *   cfg.readWriteTimeout    = 30000;   // 30 seconds
 *   OSSClient client(cfg);
 * @endcode
 */
struct ALIBABACLOUD_OSS_API ClientConfiguration {

    // ---- Region & Endpoint ----

    /// The region in which the bucket is located.
    std::optional<std::string> region;

    /// The domain names that other services can use to access OSS.
    /// When set, overrides the endpoint derived from @c region.
    std::optional<std::string> endpoint;

    // ---- Credentials ----

    /// The credentials provider to use when signing requests.
    std::shared_ptr<CredentialsProvider> credentialsProvider;

    // ---- Signing ----

    /// Authentication with OSS Signature Version (e.g., "v1" or "v4").
    /// If not set, defaults to "v1".
    std::optional<std::string> signatureVersion;

    /// Custom signer instance. When set, overrides @c signatureVersion.
    std::shared_ptr<Signer> signer;

    // ---- Retry ----

    /// Specifies the maximum number of attempts an API client will call
    /// an operation that fails with a retryable error.
    std::optional<long> retryMaxAttempts;

    /// Guides how HTTP requests should be retried in case of recoverable failures.
    /// Defaults to a StandardRetryer with FullJitterBackoff if not set.
    std::shared_ptr<Retryer> retryer;

    // ---- Endpoint Styles ----

    /// Dual-stack endpoints are provided in some regions.
    /// This allows an IPv4 client and an IPv6 client to access a bucket
    /// by using the same endpoint.
    /// Set this to @c true to use a dual-stack endpoint for the requests.
    std::optional<bool> useDualStackEndpoint;

    /// You can use an internal endpoint to communicate between Alibaba Cloud
    /// services located within the same region over the internal network.
    /// You are not charged for the traffic generated over the internal network.
    /// Set this to @c true to use an internal endpoint for the requests.
    std::optional<bool> useInternalEndpoint;

    /// OSS provides the transfer acceleration feature to accelerate data
    /// transfers of data uploads and downloads across countries and regions.
    /// Set this to @c true to use an accelerate endpoint for the requests.
    std::optional<bool> useAccelerateEndpoint;

    /// If the endpoint is a CName, set this flag to @c true.
    std::optional<bool> useCName;

    /// Allows you to enable the client to use path-style addressing,
    /// i.e., @c https://oss-cn-hangzhou.aliyuncs.com/bucket/key .
    /// By default, the OSS client will use virtual hosted addressing,
    /// i.e., @c https://bucket.oss-cn-hangzhou.aliyuncs.com/key .
    std::optional<bool> usePathStyle;

    // ---- HTTP Transport ----

    /// The HTTP client to invoke API calls with.
    /// Defaults to a CurlHttpClient if not set.
    std::shared_ptr<HttpTransport> httpTransport;

    /// Connect timeout in milliseconds.
    std::optional<long> connectTimeout;

    /// Read and write timeout in milliseconds.
    std::optional<long> readWriteTimeout;

    /// Forces the endpoint to be resolved as HTTP.
    /// Set this to @c true to disable SSL.
    std::optional<bool> disableSsl;

    /// Skip server certificate verification.
    std::optional<bool> insecureSkipVerify;

    /// Enable HTTP redirect or not. Default is disabled.
    std::optional<bool> enabledRedirect;

    /// Flag of using proxy host.
    std::optional<std::string> proxyHost;

    /// The optional user specific identifier appended to the User-Agent header.
    std::optional<std::string> userAgent;

    // ---- Signing Extras ----

    /// Additional signable headers to include in the request signature.
    std::optional<std::vector<std::string>> additionalHeaders;

    // ---- Feature Flags ----

    /// Disable automatic clock skew correction.
    /// When not set (default), the SDK will detect and correct clock skew.
    std::optional<bool> disableClockSkewCorrection;

    /// Disable automatic Content-Type detection based on the object name.
    /// When not set (default), Content-Type is auto-detected for
    /// PutObject, AppendObject and InitiateMultipartUpload.
    std::optional<bool> disableAutoDetectMimeType;

    /// Disable CRC64 integrity check for uploads.
    /// When not set (default), CRC64 is checked for PutObject, AppendObject,
    /// UploadPart, Uploader.UploadFrom and Uploader.UploadFile.
    std::optional<bool> disableUploadCRC64Check;

    /// Disable CRC64 integrity check for downloads.
    /// When not set (default), CRC64 is checked for GetObjectToFile
    /// and Downloader.DownloadFile.
    std::optional<bool> disableDownloadCRC64Check;

    // ---- Async ----

    /// Custom executor for async operations, used by OSSClient::asyncCall / asyncCallback.
    /// Must be explicitly set; if unset, OSSClient::asyncCall returns a NoExecutor error.
    std::shared_ptr<Executor> executor;

    /// The async HTTP transport for OSSAsyncClient.
    /// Defaults to a CurlMultiTransport if not set.
    std::shared_ptr<AsyncHttpTransport> asyncHttpTransport;

    /// Custom scheduled executor used only by OSSAsyncClient for internal
    /// task scheduling (response dispatch, retry delays).
    /// Has no effect on OSSClient.
    /// If unset, a default implementation is created internally.
    std::shared_ptr<ScheduledExecutor> scheduledExecutor;

    /// Creates a default configuration with all fields unset.
    static ClientConfiguration loadDefault() {
        return ClientConfiguration();
    }
};

} // namespace oss2
} // namespace alibabacloud