
#pragma once

#include "ByteStreamUtils.h"
#include "ExecuteStack.h"
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
    // endpoint
    std::string endpointScheme;
    std::string endpointAuthority;

    // user-agent
    std::string userAgent;
};

struct PresignInnerOutput {
    std::string url;
    std::string method;
    std::time_t expiration;
    HeaderCollection signedHeaders;
};

using PresignInnerResult = std::variant<PresignInnerOutput, OperationError>;


struct OperationInnerOptions {
    std::optional<OStreamFactory> ostreamFactory;
    OnResponseMessage onResponseMessage;
    std::vector<std::shared_ptr<StreamObserver>> uploadObserver;
};

class ClientImpl {
  public:
    explicit ClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns);
    virtual ~ClientImpl() = default;

    OperationResult Execute(const OperationInput& input, const OperationOptions* opts = nullptr,
                            const OperationInnerOptions* innerOpts = nullptr);

    PresignInnerResult Presign(const OperationInput& input, const OperationOptions* opts = nullptr);

  private:
    void resolveConfig(const struct ClientConfiguration& config);
    std::string resolveEndpoint(const struct ClientConfiguration& config);
    AddressStyleType resolveAddressStyle(const struct ClientConfiguration& config);
    std::shared_ptr<Retryer> resolveRetryer(const struct ClientConfiguration& config);
    std::shared_ptr<HttpTransport> resolveHttpClient(const struct ClientConfiguration& config);
    std::shared_ptr<Signer> resolveSigner(const struct ClientConfiguration& config);
    std::string resolveUserAgent(const struct ClientConfiguration& config);
    int resolveFeatureFlags(const struct ClientConfiguration& config);
    void verifyOperation(const OperationInput& input, ExecuteContext& context) const;
    void applyOperationOptions(ExecuteContext& context, const OperationOptions* opts,
                               const OperationInnerOptions* innerOpts);
    std::unique_ptr<RequestMessage> applyOperationInput(ExecuteContext& context, const OperationInput& input);
    void applyOther(ExecuteContext& context, std::unique_ptr<RequestMessage>& request,
                    const OperationInnerOptions* innerOpts);

  private:
    ClientOptions options_;
    ClientInnerOptions innerOptions_;
    std::shared_ptr<ExecuteStack> executeStack_;

  public:
    // for test only
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