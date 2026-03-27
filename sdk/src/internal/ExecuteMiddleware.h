
#pragma once

#include "alibabacloud/oss2/signer/SigningContext.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"


#include <functional>
#include <map>
#include <optional>
#include <system_error>
#include <vector>


namespace alibabacloud {
namespace oss2 {
namespace internal {

struct ExecuteContext;

using ResponseMessageFn = std::function<bool(std::unique_ptr<ResponseMessage>&, ExecuteContext&)>;
using OnResponseMessage = std::vector<ResponseMessageFn>;

struct ErrorContext {
    std::error_code error;

    // extra error
    std::map<std::string, std::string> errorFields;
    std::string snapshot;
};

struct ExecuteContext {
    long retryMaxAttempts;
    OnResponseMessage onResponseMessage;

    SigningContext signingContext;

    RequestContext transportContext;

    ErrorContext errorContext;
};

class ExecuteMiddleware {
  public:
    virtual std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request,
                                                     ExecuteContext& context) = 0;
    virtual ~ExecuteMiddleware() = default;
};
} // namespace internal

} // namespace oss2
} // namespace alibabacloud