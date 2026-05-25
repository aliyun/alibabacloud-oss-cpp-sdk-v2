#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>
#include <mutex>
#include <vector>

namespace alibabacloud {
namespace oss2 {

class CRC64MockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions&,
                   RequestCallback callback) override {
        if (request->body != nullptr) {
            auto src = request->body->spanSource();
            if (src) { src->readToEnd(); }
        }

        ResponseResult result;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            requests.emplace_back(std::make_unique<RequestMessage>(*request));
            if (!responses.empty()) {
                result = std::move(responses.front());
                responses.erase(responses.begin());
            } else {
                result = TransportError{std::make_error_code(std::errc::result_out_of_range)};
            }
        }
        callback(std::move(result), std::move(request));
    }

    std::string getName() const override { return "CRC64MockAsyncTransport"; }

    std::vector<std::unique_ptr<ResponseMessage>> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    std::mutex mutex_;
};

} // namespace oss2
} // namespace alibabacloud
