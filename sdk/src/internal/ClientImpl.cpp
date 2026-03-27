
#include "ClientImpl.h"
#include "ByteStreamUtils.h"
#include "OSSUtils.h"
#include "ResponseCheckerExecuteMiddleware.h"
#include "RetryerExecuteMiddleware.h"
#include "SignerExecuteMiddleware.h"
#include "Url.h"
#include "alibabacloud/oss2/Config.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/signer/SignerV1.h"
#include "alibabacloud/oss2/signer/SignerV4.h"
#include "src/thirdparty/tinyxml2/tinyxml2.hpp"
#include "src/transport/curl/CurlHttpClient.h"
#include "src/utils/Utils.h"


#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {


static bool onServiceError(std::unique_ptr<ResponseMessage>& response, ExecuteContext& context) {
    if (response->statusCode / 100 == 2) {
        return true;
    }

    // Read from ExecuteContext.RequestContext.errorBody
    /*
    if (response->body != nullptr) {
        auto source = response->body->spanSource();
        if (response->body->length().has_value()) {
            auto size = response->body->length().value();
            data.resize(size);
            source->read(reinterpret_cast<uint8_t*>(data.data()), size);
        } else {
            auto dd = source->readToEnd();
            data = std::string(reinterpret_cast<char*>(dd.data()), data.size());
        }
        // check status
        if ((source->state() & (~std::ios::eofbit)) != 0) {
            error_code = SdkErrorCode::READ_DATA_FAIL;
        }
    }
    */

    context.errorContext.error = make_error_code(static_cast<SdkErrorCode>(response->statusCode));
    // if (!context.transportContext.errorBody.empty()) {
    //     context.extraError.snapshot = std::move(context.transportContext.errorBody);
    // }

    // read from response body
    if (context.errorContext.snapshot.empty() && response->body != nullptr) {
        std::string data;
        response->body->seekg(0, std::ios::end);
        auto len = response->body->tellg();
        response->body->seekg(0, std::ios::beg);
        data.resize(len);
        response->body->read(data.data(), len);
        context.errorContext.snapshot = std::move(data);
    }

    std::string_view sv = context.errorContext.snapshot;
    if (sv.empty()) {
        // try to get error from x-oss-err header
        if (response->headers.find("x-oss-err") != response->headers.end()) {
            context.errorContext.snapshot = utils::Base64Decode(response->headers.at("x-oss-err"));
            sv = context.errorContext.snapshot;
        }
    }

    auto errorFields = std::map<std::string, std::string>{};
    if (sv.empty()) {
        errorFields.emplace("Code", "BadErrorResponse");
        errorFields.emplace("Message", "Empty body");
    } else {
        if (response->headers.find("Content-Type") != response->headers.end() &&
            "application/json" == response->headers.at("Content-Type")) {
            errorFields.emplace("Code", "ParseJSONError");
            errorFields.emplace("Message", "Not support json body");
        } else {
            thirdparty::tinyxml2::XMLDocument doc;
            auto xml_err = doc.Parse(sv.data(), sv.size());
            // parse xml
            if (xml_err == thirdparty::tinyxml2::XML_SUCCESS) {
                thirdparty::tinyxml2::XMLElement* root = doc.RootElement();
                if (root && !std::strncmp("Error", root->Name(), 5)) {
                    thirdparty::tinyxml2::XMLElement* node = root->FirstChildElement();
                    for (; node; node = node->NextSiblingElement()) {
                        if (node->Name() && node->GetText()) {
                            errorFields.emplace(node->Name(), node->GetText());
                        }
                    }
                } else {
                    errorFields.emplace("Code", "ParseXMLError");
                    errorFields.emplace("Message",
                                        "Xml format invalid, root node name is not Error. the content is:\n" +
                                                context.errorContext.snapshot.substr(0, 255));
                }
            } else {
                errorFields.emplace("Code", "ParseXMLError:" + std::to_string(static_cast<int>(xml_err)));
                errorFields.emplace("Message", thirdparty::tinyxml2::XMLDocument::ErrorIDToName(xml_err));
            }
        }
    }
    context.errorContext.errorFields = std::move(errorFields);

    return false;
}

ClientImpl::ClientImpl(const struct ClientConfiguration& config, const ClientOptionsFns& fns) {
    // resolveConfig
    resolveConfig(config);
    for (const auto& fn : fns) {
        fn(options_);
    }

    // Inner Options
    auto url = Url(options_.endpoint);
    innerOptions_.endpointScheme = url.scheme();
    innerOptions_.endpointAuthority = url.authority();
    // If it is IP or local host, set to path-style
    auto host = url.host();
    if (!host.empty() && (host == "localhost" || isValidIp(host))) {
        options_.addressStyle = AddressStyleType::Path;
    }
    innerOptions_.userAgent = resolveUserAgent(config);

    // Build execute stack
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
}

OperationResult ClientImpl::Execute(const OperationInput& input, const OperationOptions* opts,
                                    const OperationInnerOptions* innerOpts) {
    ExecuteContext context;

    // Verify input
    verifyOperation(input, context);
    if (context.errorContext.error) {
        return OperationError(context.errorContext.error, std::move(context.errorContext.errorFields));
    }

    // Apply operation’s options
    applyOperationOptions(context, opts, innerOpts);

    // Apply input
    auto request = applyOperationInput(context, input);

    // Apply Others
    applyOther(context, request, innerOpts);

    // Send Request
    auto response = executeStack_->Execute(request, context);

    // Response
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

    // Verify input
    verifyOperation(input, context);
    if (context.errorContext.error) {
        return OperationError(context.errorContext.error, std::move(context.errorContext.errorFields));
    }

    if (options_.credentialsProvider == nullptr) {
        return OperationError(SdkErrorCode::CREDENTIALS_PROVIDER_NULL,
                              {{"Code", "IllegalArgument"}, {"Message", "Credentials provider is null."}});
    }

    // Apply operation’s options
    applyOperationOptions(context, opts, nullptr);

    // Apply input
    auto request = applyOperationInput(context, input);

    // Apply Others
    applyOther(context, request, nullptr);

    HeaderCollection signedHeaders;

    for (;;) {
        auto* provider = options_.credentialsProvider.get();
        if (provider->getAuthType() == CredentialsProvider::AuthType::ANONYMOUS) {
            break;
        }

        Credentials cred = provider->getCredentials();

        if (!cred.hasKeys()) {
            updateError(context, SdkErrorCode::CREDENTIALS_EMPTYNULL, "CredentialsError",
                        "Credentials is null or empty.");
            break;
        }

        context.signingContext.credentials = std::move(cred);
        context.signingContext.request = request.get();
        context.signingContext.authMethodQuery = true;

        if (!options_.signer->sign(context.signingContext)) {
            updateError(context, SdkErrorCode::SIGN_ERROR, "SignatureError",
                        "The signer encountered an error while signing.");
            break;
        }

        // signed headers
        const bool isV4 = options_.signer->getName().find("v4") != std::string::npos;
        std::set<std::string, caseInsensitiveLess> additionalKeys;
        if (isV4) {
            for (const auto& k : options_.additionalHeaders) {
                additionalKeys.emplace(k);
            }
        }

        for (auto& [k, v] : request->headers) {
            const auto low = utils::ToLower(k.c_str());
            // content-type, content-md5, x-oss-
            if (low == "content-type" || low == "content-md5" || (std::strncmp(low.c_str(), "x-oss-", 6) == 0) ||
                (additionalKeys.find(low) != additionalKeys.end())) {
                signedHeaders.emplace(k, v);
            }
        }

        break;
    }

    // Response
    if (context.errorContext.error) {
        return OperationError{input.opName, std::move(request->method), std::move(request->uri),
                              context.errorContext.error, std::move(context.errorContext.errorFields)};
    }
    return PresignInnerOutput{std::move(request->uri), request->method, context.signingContext.expirationInEpoch,
                                std::move(signedHeaders)};
}


void ClientImpl::resolveConfig(const struct ClientConfiguration& config) {
    options_.product = defaults::PRODUCT;
    options_.region = config.region.value_or("");
    options_.endpoint = resolveEndpoint(config);
    options_.credentialsProvider = config.credentialsProvider;
    options_.httpTransport = resolveHttpClient(config);
    options_.signer = resolveSigner(config);
    options_.retryer = resolveRetryer(config);
    options_.addressStyle = resolveAddressStyle(config);
    options_.featureFlags = resolveFeatureFlags(config);
    if (config.additionalHeaders.has_value()) {
        options_.additionalHeaders = config.additionalHeaders.value();
    }
}

std::string ClientImpl::resolveEndpoint(const struct ClientConfiguration& config) {
    if (!config.region.has_value() && !config.endpoint.has_value()) {
        return {};
    }
    auto disableSsl = config.disableSsl.value_or(defaults::DISABLE_SSL);
    std::string endpoint;

    if (config.endpoint.has_value()) {
        endpoint = addScheme(config.endpoint.value(), disableSsl);
    } else if (config.region.has_value()) {
        auto type = EndpointType::Default;
        if (config.useDualStackEndpoint.value_or(false)) {
            type = EndpointType::DualStack;
        } else if (config.useInternalEndpoint.value_or(false)) {
            type = EndpointType::Internal;
        } else if (config.useAccelerateEndpoint.value_or(false)) {
            type = EndpointType::Accelerate;
        }
        endpoint = regionToEndpoint(config.region.value(), type, disableSsl);
    }

    return endpoint;
}

AddressStyleType ClientImpl::resolveAddressStyle(const struct ClientConfiguration& config) {
    auto style = AddressStyleType::VirtualHosted;
    if (config.useCName.value_or(false)) {
        style = AddressStyleType::CName;
    } else if (config.usePathStyle.value_or(false)) {
        style = AddressStyleType::Path;
    }

    return style;
}


std::shared_ptr<Retryer> ClientImpl::resolveRetryer(const struct ClientConfiguration& config) {
    if (config.retryer != nullptr) {
        return config.retryer;
    }

    return std::make_shared<StandardRetryer>(
            config.retryMaxAttempts.value_or(defaults::MAX_ATTEMPTS),
            std::make_unique<FullJitterBackoff>(defaults::BASE_DELAY, defaults::MAX_BACKOFF));
}

std::shared_ptr<HttpTransport> ClientImpl::resolveHttpClient(const struct ClientConfiguration& config) {
    if (config.httpTransport != nullptr) {
        return config.httpTransport;
    }

    auto httpConfig = HttpTransportOptions();
    httpConfig.connectTimeout = config.connectTimeout;
    httpConfig.readWriteTimeout = config.readWriteTimeout;
    httpConfig.insecureSkipVerify = config.insecureSkipVerify;
    httpConfig.enabledRedirect = config.enabledRedirect;
    httpConfig.proxyHost = config.proxyHost;

    return std::make_shared<transport::curl::CurlHttpClient>(httpConfig);
}

std::shared_ptr<Signer> ClientImpl::resolveSigner(const struct ClientConfiguration& config) {
    if (config.signer != nullptr) {
        return config.signer;
    }

    if ("v1" == config.signatureVersion.value_or("v4")) {
        return std::make_shared<SignerV1>();
    }
    return std::make_shared<SignerV4>();
}

std::string ClientImpl::resolveUserAgent(const struct ClientConfiguration& config) {
    std::stringstream ss;
    ss << "alibabacloud-cpp-sdk-v2/";
    ss << ALIBABACLOUD_OSS_VERSION_STR;
    // Append httpclient name
    if (options_.httpTransport != nullptr) {
        ss << "/" << options_.httpTransport->getName();
    }
    if (config.userAgent.has_value()) {
        ss << "/" << config.userAgent.value();
    }
    return ss.str();
}

int ClientImpl::resolveFeatureFlags(const struct ClientConfiguration& config) {
    ((void) (config));
    auto value = defaults::FEATURE_FLAGS;

    return value;
}


void ClientImpl::verifyOperation(const OperationInput& input, ExecuteContext& context) const{
    // check endpoint
    if (innerOptions_.endpointAuthority.empty()) {
        updateError(context, SdkErrorCode::ENDPOINT_INVALID, "IllegalArgument", "endpoint or region is invalid");
        return;
    }

    // check method
    if (!isValidMethod(input.method)) {
        updateError(context, SdkErrorCode::REQUEST_METHOD_EMPTY, "IllegalArgument",
                    "input.method is empty or invalid, got " + input.method + ".");
        return;
    }

    // check bucket name
    if (input.bucket.has_value() && !isValidBucketName(input.bucket.value())) {
        updateError(context, SdkErrorCode::REQUEST_METHOD_EMPTY, "IllegalArgument",
                    "input.bucket is invalid, got " + input.bucket.value() + ".");
        return;
    }

    // check object name
    if (input.key.has_value() && !isValidObjectName(input.key.value())) {
        updateError(context, SdkErrorCode::OBJECT_NAME_INVALID, "IllegalArgument",
                    "input.key is invalid, got " + input.key.value() + ".");
    }
}

void ClientImpl::applyOperationOptions(ExecuteContext& context, const OperationOptions* opts,
                                       const OperationInnerOptions* innerOpts) {
    // default api options
    if (opts == nullptr) {
        const static OperationOptions defaultOpts = {};
        opts = &defaultOpts;
    }

    if (innerOpts == nullptr) {
        const static OperationInnerOptions defaultInnerOpts = {};
        innerOpts = &defaultInnerOpts;
    }

    context.retryMaxAttempts = opts->retryMaxAttempts.value_or(options_.retryer->getMaxAttempts());

    // response handlers
    context.onResponseMessage.emplace_back(onServiceError);
    for (const auto& fn : innerOpts->onResponseMessage) {
        context.onResponseMessage.emplace_back(fn);
    }

    // signing context
    context.signingContext.product = options_.product;//std::string_view(options_.product);
    context.signingContext.region = options_.region;//std::string_view(options_.region);
    context.signingContext.authMethodQuery = false;

    // Transport Context
    if (innerOpts->ostreamFactory.has_value()) {
        context.transportContext.ostreamFactory = innerOpts->ostreamFactory;
    }
}

std::unique_ptr<RequestMessage> ClientImpl::applyOperationInput(ExecuteContext& context, const OperationInput& input) {
    // signing resource
    if (input.bucket.has_value()) {
        context.signingContext.bucket = input.bucket.value();
    }
    if (input.key.has_value()) {
        context.signingContext.key = input.key.value();
    }

    // request
    // request::host & path & query
    std::stringstream uri;
    uri << innerOptions_.endpointScheme << "://";
    uri << buildHostPath(input, innerOptions_.endpointAuthority, options_.addressStyle);
    auto query = utils::ToQueryString(input.parameters);
    if (!query.empty()) {
        uri << "?" << query;
    }

    // request::headers
    auto request = std::make_unique<RequestMessage>();
    auto headers = input.headers;
    headers.insert_or_assign("User-Agent", innerOptions_.userAgent);

    // signing time from user
    // x-oss-date, only support GMT time formate
    context.signingContext.clockOffset = std::chrono::seconds(0);
    context.signingContext.signTimeInEpoch = 0;

    // expiration time
    context.signingContext.expirationInEpoch = 0;
    if (input.opMetadata.find("EXPIRATION_TIME") != input.opMetadata.end()) {
        auto value = input.opMetadata.at("EXPIRATION_TIME");
        if (auto* val = std::get_if<std::int64_t>(&value)) {
            context.signingContext.expirationInEpoch = static_cast<std::time_t>(*val);
        }
    }

    // request
    request->method = input.method;
    request->uri = uri.str();
    request->headers = std::move(headers);
    request->body = input.body;

    return request;
}

void ClientImpl::applyOther(ExecuteContext& context, std::unique_ptr<RequestMessage>& request,
                            const OperationInnerOptions* innerOpts) {
    ((void) (context));
    if (innerOpts == nullptr) {
        return;
    }

    // body Stream Observer
    if (request->body != nullptr && !innerOpts->uploadObserver.empty()) {
        request->body = std::make_shared<TeeByteContent>(request->body, innerOpts->uploadObserver);
    }

    // Cancellation Callback
}


} // namespace internal
} // namespace oss2
} // namespace alibabacloud