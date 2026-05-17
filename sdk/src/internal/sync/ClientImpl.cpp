
#include "ClientImpl.h"
#include "src/internal/OSSUtils.h"
#include "ResponseCheckerExecuteMiddleware.h"
#include "RetryerExecuteMiddleware.h"
#include "SignerExecuteMiddleware.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/Error.h"
#include "src/transport/HttpTransportFactory.h"
#include "src/utils/Utils.h"

#include <cstring>
#include <set>

namespace alibabacloud {
namespace oss2 {
namespace internal {

ClientImpl::ClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns) {
    ClientOptionsFns allFns;
    allFns.reserve(fns.size() + 1);
    allFns.push_back([&config](ClientOptions& opts) {
        if (config.httpTransport != nullptr) {
            opts.httpTransport = config.httpTransport;
        } else {
            auto httpConfig = HttpTransportOptions();
            httpConfig.connectTimeout = config.connectTimeout;
            httpConfig.readWriteTimeout = config.readWriteTimeout;
            httpConfig.insecureSkipVerify = config.insecureSkipVerify;
            httpConfig.enabledRedirect = config.enabledRedirect;
            httpConfig.proxyHost = config.proxyHost;
            opts.httpTransport = transport::HttpTransportFactory::create(httpConfig);
        }
    });
    allFns.insert(allFns.end(), fns.begin(), fns.end());
    init(config, allFns);
    executeStack_ = std::make_unique<ExecuteStack>(
            [transport = options_.httpTransport]() -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<TransportExecuteMiddleware>(transport);
            });

    executeStack_->Push(
            [retryer = options_.retryer](
                    std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<RetryerExecuteMiddleware>(std::move(handle), retryer);
            },
            "Retryer");

    executeStack_->Push(
            [signer = options_.signer, provider = options_.credentialsProvider](
                    std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<SignerExecuteMiddleware>(std::move(handle), signer, provider);
            },
            "Signer");

    executeStack_->Push(
            [](std::unique_ptr<ExecuteMiddleware> handle) -> std::unique_ptr<ExecuteMiddleware> {
                return std::make_unique<ResponseCheckerExecuteMiddleware>(std::move(handle));
            },
            "ResponseChecker");

    executeStack_->Apply();
}

OperationResult ClientImpl::Execute(const OperationInput& input, const OperationOptions* opts,
                                    const OperationInnerOptions* innerOpts) {
    ExecuteContext context;

    verifyOperation(input, context);
    if (context.errorContext.error) {
        return OperationError(context.errorContext.error, std::move(context.errorContext.errorFields));
    }

    applyOperationOptions(context, opts, innerOpts);

    auto request = applyOperationInput(context, input);

    applyOther(context, request, innerOpts);

    auto response = executeStack_->Execute(request, context);

    if (context.errorContext.error || response == nullptr) {
        auto ret = OperationError{input.opName, std::move(request->method), std::move(request->uri),
                                  context.errorContext.error, std::move(context.errorContext.errorFields)};
        if (response != nullptr) {
            ret.setResponseResult(static_cast<int>(response->statusCode), std::move(response->headers),
                                  std::move(context.errorContext.snapshot));
        }
        return ret;
    }

    return OperationOutput{
            static_cast<int>(response->statusCode),
            std::move(response->headers),
            std::move(response->body),
    };
}

PresignInnerResult ClientImpl::Presign(const OperationInput& input, const OperationOptions* opts) {
    ExecuteContext context;

    verifyOperation(input, context);
    if (context.errorContext.error) {
        return OperationError(context.errorContext.error, std::move(context.errorContext.errorFields));
    }

    if (options_.credentialsProvider == nullptr) {
        return OperationError(CredentialsErrorCode::ProviderNull,
                              {{"Code", "IllegalArgument"}, {"Message", "Credentials provider is null."}});
    }

    applyOperationOptions(context, opts, nullptr);

    auto request = applyOperationInput(context, input);

    applyOther(context, request, nullptr);

    HeaderCollection signedHeaders;

    for (;;) {
        auto* provider = options_.credentialsProvider.get();
        if (provider->getAuthType() == CredentialsProvider::AuthType::ANONYMOUS) {
            break;
        }

        Credentials cred = provider->getCredentials();

        if (!cred.hasKeys()) {
            updateError(context, CredentialsErrorCode::Empty, "CredentialsError",
                        "Credentials is null or empty.");
            break;
        }

        context.signingContext.credentials = std::move(cred);
        context.signingContext.request = request.get();
        context.signingContext.authMethodQuery = true;

        if (!options_.signer->sign(context.signingContext)) {
            updateError(context, SignerErrorCode::SignFailed, "SignatureError",
                        "The signer encountered an error while signing.");
            break;
        }

        const bool isV4 = options_.signer->getName().find("v4") != std::string::npos;
        std::set<std::string, caseInsensitiveLess> additionalKeys;
        if (isV4) {
            for (const auto& k : options_.additionalHeaders) {
                additionalKeys.emplace(k);
            }
        }

        for (auto& [k, v] : request->headers) {
            const auto low = utils::ToLower(k.c_str());
            if (low == "content-type" || low == "content-md5" || (std::strncmp(low.c_str(), "x-oss-", 6) == 0) ||
                (additionalKeys.find(low) != additionalKeys.end())) {
                signedHeaders.emplace(k, v);
            }
        }

        break;
    }

    if (context.errorContext.error) {
        return OperationError{input.opName, std::move(request->method), std::move(request->uri),
                              context.errorContext.error, std::move(context.errorContext.errorFields)};
    }
    return PresignInnerOutput{std::move(request->uri), request->method, context.signingContext.expirationInEpoch,
                                std::move(signedHeaders)};
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
