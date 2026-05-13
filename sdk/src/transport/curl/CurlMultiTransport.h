#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <curl/curl.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace alibabacloud::oss2::transport::curl {

struct AsyncTransferContext;

class CurlMultiTransport : public AsyncHttpTransport {
  public:
    explicit CurlMultiTransport(const struct HttpTransportOptions& options);
    ~CurlMultiTransport() override;

    void sendAsync(std::unique_ptr<RequestMessage> request,
                   RequestContext context,
                   RequestCallback callback) override;

    std::string getName() const override {
        return "curl-multi";
    }

  private:
    void ioLoop();
    void drainPending();
    void processCompleted();
    void cleanupInflight();
    void setupCurlHandle(AsyncTransferContext* ctx);
    static void cleanupTransferContext(AsyncTransferContext* ctx);

    CURLM* multiHandle_{};
    std::thread ioThread_;
    std::atomic<bool> stopped_{false};

    std::mutex pendingMutex_;
    std::vector<std::unique_ptr<AsyncTransferContext>> pendingRequests_;

    bool verifySSL_{true};
    std::string proxyHost_;
    unsigned int proxyPort_{};
    std::string proxyUserName_;
    std::string proxyPassword_;

    long connectTimeout_{5000};
    long requestTimeout_{10000};
};

} // namespace alibabacloud::oss2::transport::curl
