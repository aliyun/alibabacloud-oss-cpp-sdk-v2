
#pragma once

#include "alibabacloud/oss2/transport/HttpTypes.h"


#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API HttpTransport {
  public:
    virtual ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) = 0;
    virtual std::string getName() const = 0;
    virtual ~HttpTransport() = default;
};

class NopHttpTransport : public HttpTransport {
  public:
    NopHttpTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override;
    std::string getName() const override {
        return "NopHttpTransport";
    }
};

struct ALIBABACLOUD_OSS_API HttpTransportOptions {
    std::optional<long> connectTimeout;
    std::optional<long> readWriteTimeout;
    std::optional<bool> insecureSkipVerify;
    std::optional<bool> enabledRedirect;
    std::optional<std::string> proxyHost;
};

} // namespace oss2
} // namespace alibabacloud