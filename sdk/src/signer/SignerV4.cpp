
#include "alibabacloud/oss2/signer/SignerV4.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/utils/Utils.h"


#include <set>
#include <sstream>
#include <vector>

namespace alibabacloud::oss2 {


namespace signer::v4 {

using HeaderSet = std::set<std::string, caseInsensitiveLess>;

static bool isDefaultSignedHeader(const std::string& lowerKey) {
    if (lowerKey == "content-type" || lowerKey == "content-md5" || lowerKey.compare(0, 6, "x-oss-") == 0) {
        return true;
    }
    return false;
}

static HeaderSet signedAdditionalHeaders(const HeaderCollection& headers, const std::vector<std::string>& additionalHeaders) {
    HeaderSet result;
    for (auto const& key : additionalHeaders) {
        std::string lowerKey = utils::ToLower(key.c_str());
        if (isDefaultSignedHeader(lowerKey)) {
            // default signed header, skip
            continue;
        } else if (headers.find(lowerKey) != headers.end()) {
            result.emplace(lowerKey);
        }
    }

    return result;
}


static std::string toHeaderSetString(const HeaderSet& headers) {
    std::stringstream ss;
    bool isFirstParam = true;
    for (auto const& key : headers) {
        std::string lowerKey = utils::ToLower(key.c_str());
        if (isFirstParam) {
            ss << lowerKey;
        } else {
            ss << ";" << lowerKey;
        }
        isFirstParam = false;
    }
    return ss.str();
}

static std::string toResource(const std::string& bucket, const std::string& key) {
    std::string resource;
    resource.append("/");
    if (!bucket.empty()) {
        resource.append(bucket);
        resource.append("/");
    }
    if (!key.empty()) {
        resource.append(key);
    }
    return resource;
}

static std::string buildCanonicalReuqest(const std::string& method, const std::string& resource,
                                         const ParameterCollection& encodedParameters, const HeaderCollection& headers,
                                         const HeaderSet& additionalHeaders) {
    /*Version 4*/
    // HTTP Verb + "\n" +
    // Canonical URI + "\n" +
    // Canonical Query String + "\n" +
    // Canonical Headers + "\n" +
    // Additional Headers + "\n" +
    // Hashed PayLoad

    std::stringstream ss;
    // "GET" | "PUT" | "POST" | ... + "\n"
    ss << method << "\n";

    // UriEncode(<Resource>) + "\n"
    ss << utils::UrlEncodePath(resource) << "\n";

    // Canonical Query String + "\n"
    // UriEncode(<QueryParam1>) + "=" + UriEncode(<Value>) + "&" + UriEncode(<QueryParam2>) + "\n"
    char separator = '&';
    bool isFirstParam = true;
    for (auto const& param : encodedParameters) {
        if (!isFirstParam) {
            ss << separator;
        } else {
            isFirstParam = false;
        }

        ss << param.first;
        if (!param.second.empty()) {
            ss << "=" << param.second;
        }
    }
    ss << "\n";

    // Lowercase(<HeaderName1>) + ":" + Trim(<value>) + "\n" + Lowercase(<HeaderName2>) + ":" + Trim(<value>) + "\n" +
    // "\n"
    for (const auto& [k, v] : headers) {
        std::string lowerKey = utils::ToLower(k.c_str());
        std::string value = utils::Trim(v.c_str());
        if (value.empty()) {
            continue;
        }
        if (isDefaultSignedHeader(lowerKey)) {
            ss << lowerKey << ":" << value << "\n";
        } else if (additionalHeaders.find(lowerKey) != additionalHeaders.end()) {
            ss << lowerKey << ":" << value << "\n";
        }
    }
    ss << "\n";

    // Lowercase(<AdditionalHeaderName1>) + ";" + Lowercase(<AdditionalHeaderName2>) + "\n" +
    ss << toHeaderSetString(additionalHeaders);
    ss << "\n";

    // Hashed PayLoad
    if (headers.find("x-oss-content-sha256") != headers.end()) {
        ss << headers.at("x-oss-content-sha256");
    } else {
        ss << "UNSIGNED-PAYLOAD";
    }

    return ss.str();
}

static std::string LowerHexToString(const unsigned char* data, size_t size) {
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    std::stringstream ss;
    for (size_t i = 0; i < size; i++)
        ss << hex[(data[i] >> 4)] << hex[(data[i] & 0x0F)];
    return ss.str();
}

std::string buildStringToSign(const std::string& datetime, const std::string& scope, const std::string& canonical) {
    // "OSS4-HMAC-SHA256" + "\n" +
    // TimeStamp + "\n" +
    // Scope + "\n" +
    // Hex(SHA256Hash(Canonical Reuqest))
    auto hashedCalRequest = utils::HashSh256(canonical.data(), canonical.size());

    std::stringstream stringToSign;
    stringToSign << "OSS4-HMAC-SHA256"
                 << "\n"
                 << datetime << "\n"
                 << scope << "\n"
                 << hashedCalRequest;

    return stringToSign.str();
}

std::string buildSignature(const std::string& keySecrect, const std::string_view& date, const std::string_view& region,
                           const std::string_view& product, const std::string& stringToSign) {
    unsigned char out[32];
    // SigningKey

    std::string key = "aliyun_v4" + keySecrect;

    // signingDate
    utils::HmacSh256(date.data(), date.size(), key.data(), key.size(), out);

    // signingRegion
    utils::HmacSh256(region.data(), region.size(), out, 32, out);

    // signingProduct
    utils::HmacSh256(product.data(), product.size(), out, 32, out);

    // signingKey
    utils::HmacSh256("aliyun_v4_request", 17, out, 32, out);

    // Signature
    utils::HmacSh256(stringToSign.data(), stringToSign.size(), out, 32, out);


    // std::cout << "signingSecret:" << LowerHexToString(signingSecret.data(), signingSecret.size()) << std::endl;
    // std::cout << "signingDate:" << LowerHexToString(signingDate.data(), signingDate.size()) << std::endl;
    // std::cout << "signingRegion:" << LowerHexToString(signingRegion.data(), signingRegion.size()) << std::endl;
    // std::cout << "signingProduct:" << LowerHexToString(signingProduct.data(), signingProduct.size()) << std::endl;
    // std::cout << "signingKey:" << LowerHexToString(signingKey.data(), signingKey.size()) << std::endl;
    // std::cout << "signature:" << LowerHexToString(signature.data(), signature.size()) << std::endl;
    return LowerHexToString(out, 32);
}


static void authHeader(SigningContext& context) {
    RequestMessage* request = context.request;
    const Credentials& cred = context.credentials;

    std::time_t timeNow;
    if (context.signTimeInEpoch > 0) {
        timeNow = context.signTimeInEpoch;
    } else {
        timeNow = std::time(nullptr) + context.clockOffset.count();
    }

    auto datetime = utils::FormatUnixTime(timeNow, "%Y%m%dT%H%M%SZ");
    auto date = std::string_view(datetime.data(), 8); //.substr(0, 8);
    auto dateRfc2822 = utils::ToGmtTime(timeNow);

    std::stringstream ss;
    ss << date << "/" << context.region << "/" << context.product << "/aliyun_v4_request";
    auto scope = ss.str();

    request->headers.insert_or_assign("x-oss-date", datetime);
    request->headers.insert_or_assign("Date", std::move(dateRfc2822));

    // if (request->headers.find("x-oss-content-sha256") == request->headers.end()) {
    request->headers.emplace("x-oss-content-sha256", "UNSIGNED-PAYLOAD");
    //}

    if (!cred.getSessionToken().empty()) {
        context.request->headers.insert_or_assign("x-oss-security-token", cred.getSessionToken());
    }

    auto additionalHeaders = signedAdditionalHeaders(request->headers, context.additionalHeaders);

    auto encodedParameters = utils::ToEncodedParameters(request->uri);

    auto resource = toResource(context.bucket, context.key);

    auto canonicalReuqest =
            buildCanonicalReuqest(request->method, resource, encodedParameters, request->headers, additionalHeaders);
    auto stringToSign = buildStringToSign(datetime, scope, canonicalReuqest);
    auto signature = buildSignature(cred.getAccessKeySecret(), date, context.region, context.product, stringToSign);

    // std::cout << "canonicalReuqest:" << std::endl << canonicalReuqest << std::endl;
    // std::cout << "stringToSign:" << std::endl << stringToSign << std::endl;

    ss.str("");
    ss << "OSS4-HMAC-SHA256"
       << " Credential=" << cred.getAccessKeyId() << "/" << scope;
    if (!additionalHeaders.empty()) {
        ss << ",AdditionalHeaders=" << toHeaderSetString(additionalHeaders);
    }
    ss << ",Signature=" << signature;

    request->headers.emplace("Authorization", ss.str());

    context.stringToSign = std::move(stringToSign);
    context.signTimeInEpoch = timeNow;
}

static void authQuery(SigningContext& context) {
    RequestMessage* request = context.request;
    const Credentials& cred = context.credentials;

    std::time_t timeNow;
    if (context.signTimeInEpoch > 0) {
        timeNow = context.signTimeInEpoch;
    } else {
        timeNow = std::time(nullptr) + context.clockOffset.count();
    }

    std::time_t expirationTime;
    if (context.expirationInEpoch > 0) {
        expirationTime = context.expirationInEpoch;
    } else {
        expirationTime = timeNow + 15 * 60;
    }

    auto datetime = utils::FormatUnixTime(timeNow, "%Y%m%dT%H%M%SZ");
    auto date = std::string_view(datetime.data(), 8); //.substr(0, 8);
    auto expires = expirationTime - timeNow;

    std::stringstream ss;
    ss << date << "/" << context.region << "/" << context.product << "/aliyun_v4_request";
    auto scope = ss.str();

    auto additionalHeaders = signedAdditionalHeaders(request->headers, context.additionalHeaders);

    auto encodedParameters = utils::ToEncodedParameters(request->uri);
    encodedParameters.erase("x-oss-signature");
    encodedParameters.insert_or_assign("x-oss-signature-version", "OSS4-HMAC-SHA256");
    encodedParameters.insert_or_assign("x-oss-date", datetime);
    encodedParameters.insert_or_assign("x-oss-expires", std::to_string(expires));
    encodedParameters.insert_or_assign("x-oss-credential", utils::UrlEncode(cred.getAccessKeyId() + "/" + scope));

    if (!cred.getSessionToken().empty()) {
        encodedParameters.insert_or_assign("x-oss-security-token", utils::UrlEncode(cred.getSessionToken()));
    }

    if (!additionalHeaders.empty()) {
        encodedParameters.insert_or_assign("x-oss-additional-headers",
                                           utils::UrlEncode(toHeaderSetString(additionalHeaders)));
    }

    auto resource = toResource(context.bucket, context.key);

    auto canonicalReuqest =
            buildCanonicalReuqest(request->method, resource, encodedParameters, request->headers, additionalHeaders);
    auto stringToSign = buildStringToSign(datetime, scope, canonicalReuqest);
    auto signature = buildSignature(cred.getAccessKeySecret(), date, context.region, context.product, stringToSign);

    encodedParameters.insert_or_assign("x-oss-signature", std::move(signature));

    // build uri
    ss.str("");
    auto queryPos = request->uri.find("?");
    if (queryPos != std::string::npos) {
        ss << request->uri.substr(0, queryPos);
    } else {
        ss << request->uri;
    }

    if (!encodedParameters.empty()) {
        ss << "?";
    }

    bool first = true;
    for (const auto& [k, v] : encodedParameters) {
        if (!first) {
            ss << "&";
        }
        ss << k;
        if (!v.empty()) {
            ss << "=" << v;
        }
        first = false;
    }

    request->uri = ss.str();

    context.stringToSign = std::move(stringToSign);
    context.signTimeInEpoch = timeNow;
    context.expirationInEpoch = expirationTime;
}

} // namespace signer::v4


bool SignerV4::sign(SigningContext& context) {
    if (context.request == nullptr || !context.credentials.hasKeys()) {
        return false;
    }

    if (context.authMethodQuery) {
        signer::v4::authQuery(context);
    } else {
        signer::v4::authHeader(context);
    }

    return true;
}


} // namespace alibabacloud::oss2
