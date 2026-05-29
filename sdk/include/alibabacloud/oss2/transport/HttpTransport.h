
#pragma once

#include "alibabacloud/oss2/transport/HttpTypes.h"

#include <functional>
#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API HttpTransport {
  public:
    virtual ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) = 0;
    virtual std::string getName() const = 0;
    virtual ~HttpTransport() = default;
};

class ALIBABACLOUD_OSS_API AsyncHttpTransport {
  public:
    virtual void sendAsync(std::unique_ptr<RequestMessage> request, const RequestOptions& options,
                           RequestCallback callback) = 0;
    virtual std::string getName() const = 0;
    virtual ~AsyncHttpTransport() = default;
};

class NopHttpTransport : public HttpTransport {
  public:
    NopHttpTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override;
    std::string getName() const override {
        return "NopHttpTransport";
    }
};

class NopAsyncHttpTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request, const RequestOptions& options,
                   RequestCallback callback) override;
    std::string getName() const override {
        return "NopAsyncHttpTransport";
    }
};

struct ALIBABACLOUD_OSS_API HttpTransportOptions {
    // Connection timeout in milliseconds, default 5s (kDefaultConnectTimeoutMs)
    std::optional<long> connectTimeout;
    // Read/write timeout in milliseconds, default 10s (kDefaultReadWriteTimeoutMs)
    std::optional<long> readWriteTimeout;
    // Skip SSL certificate verification
    std::optional<bool> insecureSkipVerify;
    // Enable HTTP redirect following
    std::optional<bool> enabledRedirect;
    // Proxy host URL, e.g. "http://proxy.example.com"
    std::optional<std::string> proxyHost;
    // Client-level check: returns true when request processing is disabled
    std::function<bool()> isRequestDisabled;
};

} // namespace oss2
} // namespace alibabacloud