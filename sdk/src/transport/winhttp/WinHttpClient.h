#pragma once

#include "WinHttpHelper.h"

namespace alibabacloud::oss2::transport::winhttp {

class WinHttpClient : public HttpTransport {
  public:
    explicit WinHttpClient(const HttpTransportOptions& options);
    explicit WinHttpClient(const WinHttpTransportOptions& options);
    ~WinHttpClient() override;

    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override;

    std::string getName() const override {
        return "winhttp";
    }

  private:
    WinHttpHandle hSession_;
    ConnectionOptions connOpts_;
};

} // namespace alibabacloud::oss2::transport::winhttp
