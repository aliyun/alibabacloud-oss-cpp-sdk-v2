
#include "ClientImplBase.h"
#include "ByteStreamUtils.h"
#include "OSSUtils.h"
#include "Url.h"
#include "alibabacloud/oss2/Config.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/signer/SignerV1.h"
#include "alibabacloud/oss2/signer/SignerV4.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "src/utils/Tinyxml2Utils.h"
#include "src/utils/Utils.h"

#include <cstdlib>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace internal {

// cppcheck-suppress constParameterCallback
static bool onPreServiceError(std::unique_ptr<ResponseMessage>& response, ExecuteContext& context) {
    if (response->statusCode / 100 == 2) {
        return true;
    }

    context.errorContext.error = make_server_error_code(response->statusCode);

    // read from response body
    if (context.errorContext.snapshot.empty() && response->body != nullptr) {
        std::string data;
        response->body->seekg(0, std::ios::end);
        auto len = response->body->tellg();
        response->body->seekg(0, std::ios::beg);
        if (len > 0) {
            data.resize(static_cast<size_t>(len));
            response->body->read(data.data(), len);
        }
        context.errorContext.snapshot = std::move(data);
    }

    std::string_view sv = context.errorContext.snapshot;
    if (sv.empty()) {
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
        if (response->headers.find("Content-Type") != response->headers.end()
            && "application/json" == response->headers.at("Content-Type")) {
            errorFields.emplace("Code", "ParseJSONError");
            errorFields.emplace("Message", "Not support json body");
        } else {
            thirdparty::tinyxml2::XMLDocument doc;
            auto xml_err = doc.Parse(sv.data(), sv.size());
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
                                        "Xml format invalid, root node name is not Error. the content is:\n"
                                            + context.errorContext.snapshot.substr(0, 255));
                }
            } else {
                errorFields.emplace("Code", "ParseXMLError:" + std::to_string(static_cast<int>(xml_err)));
                errorFields.emplace("Message", thirdparty::tinyxml2::XMLDocument::ErrorIDToName(xml_err));
            }
        }
    }
    context.errorContext.errorFields = std::move(errorFields);

    return true;
}

// cppcheck-suppress constParameterCallback
static bool onPostServiceError([[maybe_unused]] std::unique_ptr<ResponseMessage>& response, ExecuteContext& context) {
    return !context.errorContext.error;
}

// cppcheck-suppress constParameterReference
static bool onClockSkewError(std::unique_ptr<ResponseMessage>& response, ExecuteContext& context,
                             int64_t& clockOffset) {
    bool isClockSkewError = false;

    if (response->statusCode == 403) {
        auto it = context.errorContext.errorFields.find("Code");
        if (it != context.errorContext.errorFields.end() && it->second == "RequestTimeTooSkewed") {
            isClockSkewError = true;
        }
    } else if (response->statusCode == 400) {
        auto codeIt = context.errorContext.errorFields.find("Code");
        auto msgIt = context.errorContext.errorFields.find("Message");
        if (codeIt != context.errorContext.errorFields.end() && codeIt->second == "InvalidArgument"
            && msgIt != context.errorContext.errorFields.end()
            && msgIt->second == "Invalid signing date in Authorization header.") {
            isClockSkewError = true;
        }
    }

    if (!isClockSkewError) {
        return true;
    }

    std::time_t serverTime = -1;
    auto dateIt = response->headers.find("Date");
    if (dateIt != response->headers.end()) {
        serverTime = utils::GmtToUnixTime(dateIt->second);
    }

    if (serverTime == -1) {
        auto stIt = context.errorContext.errorFields.find("ServerTime");
        if (stIt != context.errorContext.errorFields.end()) {
            serverTime = utils::UtcToUnixTime(stIt->second);
        }
    }

    if (serverTime == -1) {
        return true;
    }

    auto localTime = std::time(nullptr);
    auto newOffset = static_cast<int64_t>(serverTime - localTime);

    if (response->statusCode == 400 && std::abs(newOffset) < 900) {
        return true;
    }

    if (std::abs(newOffset - clockOffset) > 60) {
        clockOffset = newOffset;
    }
    context.signingContext.clockOffset = std::chrono::seconds(newOffset);

    context.errorContext.error = make_retryable_server_error_code(response->statusCode);
    return true;
}

void ClientImplBase::init(const struct ClientConfiguration& config, const ClientOptionsFns& fns) {
    resolveConfig(config);
    for (const auto& fn : fns) {
        fn(options_);
    }

    auto url = Url(options_.endpoint);
    innerOptions_.endpointScheme = url.scheme();
    innerOptions_.endpointAuthority = url.authority();
    auto host = url.host();
    if (!host.empty() && (host == "localhost" || isValidIp(host))) {
        options_.addressStyle = AddressStyleType::Path;
    }
    innerOptions_.userAgent = resolveUserAgent(config);

    // deferred initialization error: surfaced at operation-execute time, not construction
    if (config.accountId.has_value() && !config.accountId.value().empty() &&
        !isValidAccountId(config.accountId.value())) {
        innerOptions_.initError = make_error_code(ClientErrorCode::ArgumentInvalid);
        innerOptions_.initErrorFields = {
            {"Code", "IllegalArgument"},
            {"Message", "invalid account id: " + config.accountId.value() + ", must be pure digits"}};
    }
}

void ClientImplBase::resolveConfig(const struct ClientConfiguration& config) {
    options_.product = defaults::PRODUCT;
    options_.region = config.region.value_or("");
    options_.accountId = config.accountId.value_or("");
    options_.endpoint = resolveEndpoint(config);
    options_.credentialsProvider = config.credentialsProvider;
    options_.signer = resolveSigner(config);
    options_.retryer = resolveRetryer(config);
    options_.addressStyle = resolveAddressStyle(config);
    options_.featureFlags = resolveFeatureFlags(config);
    if (config.additionalHeaders.has_value()) {
        options_.additionalHeaders = config.additionalHeaders.value();
    }
}

std::string ClientImplBase::resolveEndpoint(const struct ClientConfiguration& config) {
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

AddressStyleType ClientImplBase::resolveAddressStyle(const struct ClientConfiguration& config) {
    auto style = AddressStyleType::VirtualHosted;
    if (config.useCName.value_or(false)) {
        style = AddressStyleType::CName;
    } else if (config.usePathStyle.value_or(false)) {
        style = AddressStyleType::Path;
    }
    return style;
}

std::shared_ptr<Retryer> ClientImplBase::resolveRetryer(const struct ClientConfiguration& config) {
    if (config.retryer != nullptr) {
        return config.retryer;
    }
    return std::make_shared<StandardRetryer>(
        config.retryMaxAttempts.value_or(defaults::MAX_ATTEMPTS),
        std::make_unique<FullJitterBackoff>(defaults::BASE_DELAY, defaults::MAX_BACKOFF));
}

std::shared_ptr<Signer> ClientImplBase::resolveSigner(const struct ClientConfiguration& config) {
    if (config.signer != nullptr) {
        return config.signer;
    }
    if ("v1" == config.signatureVersion.value_or("v4")) {
        return std::make_shared<SignerV1>();
    }
    return std::make_shared<SignerV4>();
}

std::string ClientImplBase::resolveUserAgent(const struct ClientConfiguration& config) {
    std::stringstream ss;
    ss << "alibabacloud-cpp-sdk-v2/";
    ss << ALIBABACLOUD_OSS_SDK_VERSION_STR;
    if (options_.httpTransport != nullptr) {
        ss << "/" << options_.httpTransport->getName();
    } else if (options_.asyncHttpTransport != nullptr) {
        ss << "/" << options_.asyncHttpTransport->getName();
    }
    if (config.userAgent.has_value()) {
        ss << "/" << config.userAgent.value();
    }
    return ss.str();
}

int ClientImplBase::resolveFeatureFlags(const struct ClientConfiguration& config) {
    int flags = defaults::FEATURE_FLAGS;
    if (config.disableClockSkewCorrection.value_or(false)) {
        flags &= ~static_cast<int>(FeatureFlagsType::CorrectClockSkew);
    }
    if (config.disableAutoDetectMimeType.value_or(false)) {
        flags &= ~static_cast<int>(FeatureFlagsType::AutoDetectMimeType);
    }
    if (config.disableUploadCRC64Check.value_or(false)) {
        flags &= ~static_cast<int>(FeatureFlagsType::EnableCRC64CheckUpload);
    }
    if (config.disableDownloadCRC64Check.value_or(false)) {
        flags &= ~static_cast<int>(FeatureFlagsType::EnableCRC64CheckDownload);
    }
    return flags;
}

void ClientImplBase::verifyOperation(const OperationInput& input, ExecuteContext& context) const {
    if (innerOptions_.endpointAuthority.empty()) {
        updateError(context, ClientErrorCode::EndpointInvalid, "IllegalArgument", "endpoint or region is invalid");
        return;
    }
    if (!isValidMethod(input.method)) {
        updateError(context, ClientErrorCode::RequestMethodEmpty, "IllegalArgument",
                    "input.method is empty or invalid, got " + input.method + ".");
        return;
    }
    if (input.bucket.has_value() && !isValidBucketName(input.bucket.value())) {
        updateError(context, ClientErrorCode::BucketNameInvalid, "IllegalArgument",
                    "input.bucket is invalid, got " + input.bucket.value() + ".");
        return;
    }
    if (input.key.has_value() && !isValidObjectName(input.key.value())) {
        updateError(context, ClientErrorCode::ObjectNameInvalid, "IllegalArgument",
                    "input.key is invalid, got " + input.key.value() + ".");
    }
}

void ClientImplBase::applyOperationOptions(ExecuteContext& context, const OperationOptions* opts,
                                           const OperationInnerOptions* innerOpts) {
    if (opts == nullptr) {
        const static OperationOptions defaultOpts = {};
        opts = &defaultOpts;
    }
    if (innerOpts == nullptr) {
        const static OperationInnerOptions defaultInnerOpts = {};
        innerOpts = &defaultInnerOpts;
    }

    context.retryMaxAttempts = opts->retryMaxAttempts.value_or(options_.retryer->getMaxAttempts());

    context.onResponseMessage.emplace_back(onPreServiceError);
    if (hasFlag(FeatureFlagsType::CorrectClockSkew)) {
        context.onResponseMessage.emplace_back(
            // cppcheck-suppress constParameterReference
            [&offset = innerOptions_.clockOffset](std::unique_ptr<ResponseMessage>& response, ExecuteContext& ctx) {
                return onClockSkewError(response, ctx, offset);
            });
    }
    context.onResponseMessage.emplace_back(onPostServiceError);
    for (const auto& fn : innerOpts->onResponseMessage) {
        context.onResponseMessage.emplace_back(fn);
    }

    context.signingContext.product = options_.product;
    context.signingContext.region = options_.region;
    context.signingContext.authMethodQuery = false;

    if (innerOpts->sinkFactory.has_value()) {
        context.transportContext.sinkFactory = innerOpts->sinkFactory;
    }

    if (opts->cancellationToken.has_value() && opts->cancellationToken->canBeCanceled()) {
        context.transportContext.cancellationToken = opts->cancellationToken;
    }
}

std::unique_ptr<RequestMessage> ClientImplBase::applyOperationInput(ExecuteContext& context,
                                                                    const OperationInput& input) {
    // resolve bucket name for signing only; keep input.bucket as the logical name
    if (input.bucket.has_value()) {
        context.signingContext.bucket =
            options_.bucketNameResolver ? options_.bucketNameResolver(input) : input.bucket.value();
    }
    if (input.key.has_value()) {
        context.signingContext.key = input.key.value();
    }

    auto request = std::make_unique<RequestMessage>();

    std::stringstream uri;
    if (options_.endpointProvider) {
        std::error_code ec;
        auto url = options_.endpointProvider(input, ec);
        if (ec) {
            updateError(context, ec, "IllegalArgument", "EndpointProvider returns error, detail is " + ec.message());
            return request;
        }
        uri << url;
    } else {
        uri << innerOptions_.endpointScheme << "://";
        uri << buildHostPath(input, innerOptions_.endpointAuthority, options_.addressStyle);
    }
    auto query = utils::ToQueryString(input.parameters);
    if (!query.empty()) {
        uri << "?" << query;
    }

    auto headers = input.headers;
    headers.insert_or_assign("User-Agent", innerOptions_.userAgent);

    if (innerOptions_.clockOffset != 0) {
        context.signingContext.clockOffset = std::chrono::seconds(innerOptions_.clockOffset);
    } else {
        context.signingContext.clockOffset = std::chrono::seconds(0);
    }
    context.signingContext.signTimeInEpoch = 0;

    context.signingContext.expirationInEpoch = 0;
    if (input.opMetadata.find("EXPIRATION_TIME") != input.opMetadata.end()) {
        auto value = input.opMetadata.at("EXPIRATION_TIME");
        if (auto* val = std::get_if<std::int64_t>(&value)) {
            context.signingContext.expirationInEpoch = static_cast<std::time_t>(*val);
        }
    }

    request->method = input.method;
    request->uri = uri.str();
    request->headers = std::move(headers);
    request->body = input.body;

    return request;
}

// cppcheck-suppress constParameterReference
void ClientImplBase::applyOther([[maybe_unused]] ExecuteContext& context, std::unique_ptr<RequestMessage>& request,
                                const OperationInnerOptions* innerOpts) {
    if (innerOpts == nullptr) {
        return;
    }
    if (request->body != nullptr && !innerOpts->uploadObserver.empty()) {
        request->body = std::make_shared<TeeByteContent>(request->body, innerOpts->uploadObserver);
    }
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
