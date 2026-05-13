#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSFwd.h"

#include <future>
#include <memory>

namespace alibabacloud {
namespace oss2 {

struct ClientConfiguration;

namespace internal {
class AsyncClientImpl;
}

class ALIBABACLOUD_OSS_API OSSAsyncClient final {
  public:
    explicit OSSAsyncClient(const struct ClientConfiguration& config);
    explicit OSSAsyncClient(const struct ClientConfiguration& config, ClientOptionsFns& fns);
    ~OSSAsyncClient();

    void invokeOperationAsync(const OperationInput& input,
                              const OperationCallback& callback,
                              const OperationOptions* options = nullptr);

    template<typename OutcomeT, typename RequestT, typename CallbackT>
    std::future<OutcomeT> callAsync(
            void(OSSAsyncClient::*method)(const RequestT&, const CallbackT&, const OperationOptions*),
            const RequestT& request,
            const OperationOptions* options = nullptr) {
        auto promise = std::make_shared<std::promise<OutcomeT>>();
        (this->*method)(request, CallbackT([promise](OutcomeT result) {
            promise->set_value(std::move(result));
        }), options);
        return promise->get_future();
    }

  private:
    std::shared_ptr<internal::AsyncClientImpl> client_;
};

} // namespace oss2
} // namespace alibabacloud
