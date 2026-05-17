#pragma once

#include "WinHttpHelper.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace alibabacloud::oss2::transport::winhttp {

class WinHttpAsyncClient : public AsyncHttpTransport {
  public:
    explicit WinHttpAsyncClient(const HttpTransportOptions& options);
    explicit WinHttpAsyncClient(const WinHttpTransportOptions& options);
    ~WinHttpAsyncClient() override;

    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options,
                   RequestCallback callback) override;

    std::string getName() const override {
        return "winhttp-async";
    }

  private:
    friend struct AsyncRequestContext;
    void onRequestStarted();
    void onRequestFinished();

    WinHttpHandle hSession_;
    ConnectionOptions connOpts_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<int> inflightCount_{0};
};

} // namespace alibabacloud::oss2::transport::winhttp
