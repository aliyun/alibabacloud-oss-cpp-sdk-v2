
#include "AsyncClientImpl.h"
#include "AsyncExecuteMiddleware.h"
#include "AsyncExecuteStack.h"
#include "ResponseCheckerAsyncMiddleware.h"
#include "RetryerAsyncMiddleware.h"
#include "SignerAsyncMiddleware.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/signer/Signer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/utils/ScheduledExecutor.h"
#include "src/internal/OSSUtils.h"
#include "src/transport/HttpTransportFactory.h"
#include "src/utils/DefaultScheduledExecutor.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

AsyncClientImpl::AsyncClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns) {
    ClientOptionsFns allFns;
    allFns.reserve(fns.size() + 1);
    allFns.push_back([&config](ClientOptions& opts) {
        if (config.asyncHttpTransport != nullptr) {
            opts.asyncHttpTransport = config.asyncHttpTransport;
        } else {
            auto httpConfig = HttpTransportOptions();
            httpConfig.connectTimeout = config.connectTimeout;
            httpConfig.readWriteTimeout = config.readWriteTimeout;
            httpConfig.insecureSkipVerify = config.insecureSkipVerify;
            httpConfig.proxyHost = config.proxyHost;
            opts.asyncHttpTransport = transport::AsyncHttpTransportFactory::create(httpConfig);
        }
    });
    allFns.insert(allFns.end(), fns.begin(), fns.end());
    init(config, allFns);

    scheduler_ = config.scheduledExecutor ? config.scheduledExecutor : std::make_shared<DefaultScheduledExecutor>();

    stack_ = std::make_shared<AsyncExecuteStack>(
        options_.asyncHttpTransport,
        [this](const std::shared_ptr<AsyncExecuteState>& state) { onOperationFinished(state); });

    stack_->Push(
        [retryer = options_.retryer](
            std::unique_ptr<AsyncExecuteMiddleware> next) -> std::unique_ptr<AsyncExecuteMiddleware> {
            return std::make_unique<RetryerAsyncMiddleware>(std::move(next), retryer);
        },
        "Retryer");

    stack_->Push(
        [signer = options_.signer, provider = options_.credentialsProvider](
            std::unique_ptr<AsyncExecuteMiddleware> next) -> std::unique_ptr<AsyncExecuteMiddleware> {
            return std::make_unique<SignerAsyncMiddleware>(std::move(next), signer, provider);
        },
        "Signer");

    stack_->Push(
        [](std::unique_ptr<AsyncExecuteMiddleware> next) -> std::unique_ptr<AsyncExecuteMiddleware> {
            return std::make_unique<ResponseCheckerAsyncMiddleware>(std::move(next));
        },
        "ResponseChecker");

    stack_->Apply();
}

AsyncClientImpl::~AsyncClientImpl() {
    options_.asyncHttpTransport = nullptr;
}

void AsyncClientImpl::ExecuteAsync(const OperationInput& input, OperationCallback callback,
                                   const OperationOptions* opts, const OperationInnerOptions* innerOpts) {
    ExecuteContext context;

    verifyOperation(input, context);
    if (context.errorContext.error) {
        callback(OperationError(context.errorContext.error, std::move(context.errorContext.errorFields)));
        return;
    }

    applyOperationOptions(context, opts, innerOpts);

    auto request = applyOperationInput(context, input);

    applyOther(context, request, innerOpts);

    auto state = std::make_shared<AsyncExecuteState>();
    state->opName = input.opName;
    state->callback = std::move(callback);
    state->context = std::move(context);
    state->request = std::move(request);
    state->signTime = state->context.signingContext.signTimeInEpoch;
    state->expiration = state->context.signingContext.expirationInEpoch;

    stack_->executeAsync(state);
}

void AsyncClientImpl::onOperationFinished(const std::shared_ptr<AsyncExecuteState>& state) {
    if (state->action == ResponseAction::Continue) {
        scheduler_->schedule(state->retryDelay, [this, state]() { stack_->executeAsync(state); });
        return;
    }

    scheduler_->schedule(std::chrono::milliseconds(0), [state]() {
        auto buildResult = [&]() -> OperationResult {
            if (state->context.errorContext.error || state->response == nullptr) {
                auto err = OperationError{state->opName, std::move(state->request->method),
                                          std::move(state->request->uri), state->context.errorContext.error,
                                          std::move(state->context.errorContext.errorFields)};
                if (state->response != nullptr) {
                    err.setResponseResult(static_cast<int>(state->response->statusCode),
                                          std::move(state->response->headers),
                                          std::move(state->context.errorContext.snapshot));
                }
                return err;
            }
            return OperationOutput{
                static_cast<int>(state->response->statusCode),
                std::move(state->response->headers),
                std::move(state->response->body),
            };
        };
        state->callback(buildResult());
    });
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
