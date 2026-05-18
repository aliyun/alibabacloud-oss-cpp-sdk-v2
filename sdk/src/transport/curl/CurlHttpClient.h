#pragma once

#include "CurlContainer.h"
#include "CurlHelper.h"

namespace alibabacloud::oss2::transport::curl {

class CurlHttpClient : public HttpTransport {
  public:
    explicit CurlHttpClient(const HttpTransportOptions& options);
    explicit CurlHttpClient(const CurlTransportOptions& options);

    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override;

    std::string getName() const override;

  private:
    std::unique_ptr<CurlContainer> curlContainer_;
    ClientOptions clientOpts_;
};
} // namespace alibabacloud::oss2::transport::curl
