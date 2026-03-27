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

struct ALIBABACLOUD_OSS_API ClientOptions {
    std::string product;
    std::string region;
    std::string endpoint;
    std::shared_ptr<CredentialsProvider> credentialsProvider;
    std::shared_ptr<Signer> signer;
    std::shared_ptr<Retryer> retryer;
    std::shared_ptr<HttpTransport> httpTransport;
    AddressStyleType addressStyle;
    int featureFlags;
    std::vector<std::string> additionalHeaders;
};

using ClientOptionsFn = std::function<void(ClientOptions&)>;
using ClientOptionsFns = std::vector<ClientOptionsFn>;

} // namespace oss2
} // namespace alibabacloud