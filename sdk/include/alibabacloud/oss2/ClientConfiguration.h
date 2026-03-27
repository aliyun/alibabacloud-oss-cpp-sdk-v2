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

struct ALIBABACLOUD_OSS_API ClientConfiguration {
    std::optional<std::string> region;
    std::optional<std::string> endpoint;
    std::shared_ptr<CredentialsProvider> credentialsProvider;

    std::optional<std::string> signatureVersion;
    std::shared_ptr<Signer> signer;

    std::optional<long> retryMaxAttempts;
    std::shared_ptr<Retryer> retryer;

    std::optional<bool> useDualStackEndpoint;
    std::optional<bool> useInternalEndpoint;
    std::optional<bool> useAccelerateEndpoint;
    std::optional<bool> useCName;
    std::optional<bool> usePathStyle;

    std::shared_ptr<HttpTransport> httpTransport;
    std::optional<long> connectTimeout;
    std::optional<long> readWriteTimeout;
    std::optional<bool> disableSsl;
    std::optional<bool> insecureSkipVerify;
    std::optional<bool> enabledRedirect;
    std::optional<std::string> proxyHost;
    std::optional<std::string> userAgent;

    std::optional<std::vector<std::string>> additionalHeaders;

    static ClientConfiguration loadDefault() {
        return ClientConfiguration();
    }
};

} // namespace oss2
} // namespace alibabacloud