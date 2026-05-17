#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"

#include <memory>
#include <vector>

namespace alibabacloud {
namespace oss2 {

class MockTransport : public HttpTransport {
  public:
    MockTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        saveRequest(request);
        return popResponse();
    }
    std::string getName() const override {
        return "MockTransport";
    }

  public:
    std::vector<std::unique_ptr<ResponseMessage>> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    RequestMessage* lastRequest = nullptr;

    void Clear() {
        responses.clear();
        lastRequest = nullptr;
    }

  private:
    void saveRequest(std::unique_ptr<RequestMessage>& request) {
        auto req = std::make_unique<RequestMessage>(*request);
        lastRequest = req.get();
        requests.emplace_back(std::move(req));
        if (lastRequest->body != nullptr) {
            auto src = lastRequest->body->spanSource();
            src->readToEnd();
        }
    }

    ResponseResult popResponse() {
        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return TransportError{std::make_error_code(std::errc::result_out_of_range)};
    }
};

} // namespace oss2
} // namespace alibabacloud
