#pragma once

#include "CurlContainer.h"
#include "CurlHelper.h"

#include <curl/curl.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

namespace alibabacloud::oss2::transport::curl {

struct AsyncTransferContext;

class CurlMultiTransport : public AsyncHttpTransport {
  public:
    explicit CurlMultiTransport(const HttpTransportOptions& options);
    explicit CurlMultiTransport(const CurlTransportOptions& options);
    ~CurlMultiTransport() override;

    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options,
                   RequestCallback callback) override;

    std::string getName() const override;

  private:
    void ioLoop();
    void drainPending();
    void processCompleted();
    void cleanupInflight();
    void setupCurlHandle(AsyncTransferContext* ctx);

    std::unique_ptr<CurlContainer> curlContainer_;
    CURLM* multiHandle_{};
    std::thread ioThread_;
    std::atomic<bool> stopped_{false};

    std::mutex pendingMutex_;
    std::vector<std::unique_ptr<AsyncTransferContext>> pendingRequests_;

    std::unordered_set<AsyncTransferContext*> inflightHandles_;

    ClientOptions clientOpts_;
};

} // namespace alibabacloud::oss2::transport::curl
