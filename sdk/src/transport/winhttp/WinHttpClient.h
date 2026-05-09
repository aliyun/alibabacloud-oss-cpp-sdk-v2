
#pragma once

#include "WinHttpConnectionPool.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <windows.h>
#include <winhttp.h>

#include <string>

namespace alibabacloud {
namespace oss2 {
namespace transport {
namespace winhttp {

class WinHttpClient : public HttpTransport {
  public:
    WinHttpClient(const HttpTransportOptions& options);
    ~WinHttpClient() override;

    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override;

    std::string getName() const override {
        return "winhttp";
    }

  private:
    static std::wstring ToWideString(const std::string& str);
    static std::string FromWideString(const std::wstring& wstr);

    HINTERNET hSession_;
    std::unique_ptr<WinHttpConnectionPool> connectionPool_;
    bool verifySSL_;
    bool enabledRedirect_;
    std::string proxyHost_;
};

} // namespace winhttp
} // namespace transport
} // namespace oss2
} // namespace alibabacloud
