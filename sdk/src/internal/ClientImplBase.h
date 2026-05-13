
#pragma once

#include "ByteStreamUtils.h"
#include "ExecuteMiddleware.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSFwd.h"

#include <memory>
#include <string>
#include <tuple>

namespace alibabacloud {
namespace oss2 {
namespace internal {

struct ClientInnerOptions {
    std::string endpointScheme;
    std::string endpointAuthority;
    std::string userAgent;
};

struct OperationInnerOptions {
    std::optional<OStreamFactory> ostreamFactory;
    OnResponseMessage onResponseMessage;
    std::vector<std::shared_ptr<StreamObserver>> uploadObserver;
};

class ClientImplBase {
  public:
    virtual ~ClientImplBase() = default;

  protected:
    ClientImplBase() = default;

    void init(const struct ClientConfiguration& config, const ClientOptionsFns& fns);

    void verifyOperation(const OperationInput& input, ExecuteContext& context) const;
    void applyOperationOptions(ExecuteContext& context, const OperationOptions* opts,
                               const OperationInnerOptions* innerOpts);
    std::unique_ptr<RequestMessage> applyOperationInput(ExecuteContext& context, const OperationInput& input);
    void applyOther(ExecuteContext& context, std::unique_ptr<RequestMessage>& request,
                    const OperationInnerOptions* innerOpts);

    ClientOptions options_;
    ClientInnerOptions innerOptions_;

  private:
    void resolveConfig(const struct ClientConfiguration& config);
    std::string resolveEndpoint(const struct ClientConfiguration& config);
    AddressStyleType resolveAddressStyle(const struct ClientConfiguration& config);
    std::shared_ptr<Retryer> resolveRetryer(const struct ClientConfiguration& config);
    std::shared_ptr<Signer> resolveSigner(const struct ClientConfiguration& config);
    std::string resolveUserAgent(const struct ClientConfiguration& config);
    int resolveFeatureFlags(const struct ClientConfiguration& config);

  public:
    inline ClientOptions& getOptions() {
        return options_;
    }
    inline ClientInnerOptions& getInnerOptions() {
        return innerOptions_;
    }
    inline bool hasFlag(FeatureFlagsType flag) {
        return (options_.featureFlags & static_cast<int>(flag)) != 0;
    }
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
