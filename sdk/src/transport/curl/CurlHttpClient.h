#pragma once

#include "CurlContainer.h"
#include "CurlHelper.h"

namespace alibabacloud::oss2::transport::curl {

class CurlHttpClient : public HttpTransport {
  public:
    explicit CurlHttpClient(const HttpTransportOptions& options);
    explicit CurlHttpClient(const CurlTransportOptions& options);

    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override;

    std::string getName() const override {
        return "curl";
    }

  private:
    std::unique_ptr<CurlContainer> curlContainer_;
    ConnectionOptions connOpts_;
};
} // namespace alibabacloud::oss2::transport::curl
