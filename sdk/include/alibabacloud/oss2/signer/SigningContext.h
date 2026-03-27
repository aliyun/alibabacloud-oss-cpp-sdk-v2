
#pragma once

#include "alibabacloud/oss2/credentials/Credentials.h"
#include <chrono>
#include <ctime>
#include <string_view>
#include <vector>


namespace alibabacloud {
namespace oss2 {

// forward declare
struct RequestMessage;

struct ALIBABACLOUD_OSS_API SigningContext {
    /**
     * Product identifier used to generate signing scope.
     */
    std::string_view product;

    /**
     * Region identifier used to generate signing scope.
     */
    std::string_view region;

    /**
     * Bucket name used to construct the request resource path.
     */
    std::string bucket;

    /**
     * Object key used to construct the request resource path.
     */
    std::string key;

    /**
     * Indicates whether query-based authentication should be used.
     */
    bool authMethodQuery;

    /**
     * The credential object used for signing.
     */
    Credentials credentials;

    /**
     * The current request object containing HTTP method, URI, and headers.
     */
    RequestMessage* request;

    /**
     * The generated canonical string to be signed (for debugging or logging).
     */
    std::string stringToSign;

    /**
     * The expiration time of the signature.
     * It is the epoch number of seconds from the epoch instant 1970-01-01T00:00:00Z
     */
    std::time_t expirationInEpoch;

    /**
     * The timestamp when the signature was created.
     * It is the epoch number of seconds from the epoch instant 1970-01-01T00:00:00Z
     */
    std::time_t signTimeInEpoch;

    /**
     * The difference between server time and local time
     */
    std::chrono::seconds clockOffset;

    /**
     * Additional list of HTTP header fields that should participate in signing.
     */
    std::vector<std::string> additionalHeaders;

    /**
     * List of sub-resource parameters such as "acl", "versionId".
     */
    // List<String> subResource;
};
} // namespace oss2
} // namespace alibabacloud
