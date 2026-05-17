#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>
#include <mutex>
#include <vector>

namespace alibabacloud {
namespace oss2 {

class MockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions&,
                   RequestCallback callback) override {
        ResponseResult responseResult = TransportError{std::make_error_code(std::errc::result_out_of_range)};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto req = std::make_unique<RequestMessage>(*request);
            lastRequest = req.get();
            requests.emplace_back(std::move(req));

            if (!responses.empty()) {
                responseResult = std::move(responses.front());
                responses.erase(responses.begin());
            }
        }
        callback(std::move(responseResult), std::move(request));
    }

    std::string getName() const override { return "MockAsyncTransport"; }

    std::vector<std::unique_ptr<ResponseMessage>> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    RequestMessage* lastRequest = nullptr;
    std::mutex mutex_;

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        responses.clear();
        requests.clear();
        lastRequest = nullptr;
    }
};

} // namespace oss2
} // namespace alibabacloud
