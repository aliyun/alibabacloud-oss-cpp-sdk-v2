#pragma once

#include "CurlContainer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

namespace alibabacloud::oss2::transport::curl {

class CurlHttpClient : public HttpTransport {
  public:
    CurlHttpClient(const struct HttpTransportOptions& options);

    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override;

    std::string getName() const override {
        return "curl";
    }

  private:
    std::unique_ptr<CurlContainer> curlContainer_;
    std::string proxyHost_;
    unsigned int proxyPort_{};
    std::string proxyUserName_;
    std::string proxyPassword_;
    bool verifySSL_{};
    std::string caPath_;
    std::string caFile_;
    std::string networkInterface_;
};
} // namespace alibabacloud::oss2::transport::curl