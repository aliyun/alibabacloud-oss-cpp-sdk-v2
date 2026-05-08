#include "alibabacloud/oss2/signer/SignerV1.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/utils/Utils.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace alibabacloud::oss2 {


namespace signer::v1 {

std::string buildResource(const std::string& bucket, const std::string& key) {
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

const static std::set<std::string> SubResourceSet = {
        "acl",
        "bucketInfo",
        "location",
        "stat",
        "delete",
        "append",
        "tagging",
        "objectMeta",
        "uploads",
        "uploadId",
        "partNumber",
        "security-token",
        "position",
        "response-content-type",
        "response-content-language",
        "response-expires",
        "response-cache-control",
        "response-content-disposition",
        "response-content-encoding",
        "restore",
        "callback",
        "callback-var",
        "versions",
        "versioning",
        "versionId",
        "sequential",
        "continuation-token",
        "regionList",
        "cloudboxes",
        "symlink",
        "resourceGroup",
        "cleanRestoredObject",
};

static bool isSignHeader(const std::string& key) {
    return key.compare(0, 6, "x-oss-") == 0;
}

static std::string buildCanonicalHeaders(const HeaderCollection& headers) {
    std::map<std::string, std::string> canonicalMap;
    for (const auto& [k, v] : headers) {
        if (v.empty()) {
            continue;
        }
        std::string lowerKey = utils::ToLower(k.c_str());
        if (isSignHeader(lowerKey)) {
            canonicalMap[lowerKey] = v;
        }
    }

    std::stringstream ss;
    for (const auto& [k, v] : canonicalMap) {
        ss << k << ":" << v << "\n";
    }
    return ss.str();
}

static std::string getDateFromHeaders(const HeaderCollection& headers) {
    if (headers.find("x-oss-date") != headers.end()) {
        return headers.at("x-oss-date");
    }

    if (headers.find("Date") != headers.end()) {
        return headers.at("Date");
    }

    return "";
}

static std::string calcStringToSign(SigningContext& context, const std::string& dateOverride = "") {
    RequestMessage* request = context.request;

    std::string canonicalUri = buildResource(context.bucket, context.key);

    std::string canonicalQuery;
    if (request != nullptr) {
        auto encodedParameters = utils::ToEncodedParameters(request->uri);
        std::vector<std::pair<std::string, std::string>> sortedEntries;
        for (const auto& [k, v] : encodedParameters) {
            sortedEntries.emplace_back(k, v);
        }
        std::sort(sortedEntries.begin(), sortedEntries.end());

        std::vector<std::string> queryParts;
        for (const auto& [k, v] : sortedEntries) {
            std::string decodedKey = utils::UrlDecode(k);
            std::string decodedValue = utils::UrlDecode(v);
            if (SubResourceSet.count(decodedKey) > 0) {
                if (!decodedValue.empty()) {
                    queryParts.push_back(decodedKey + "=" + decodedValue);
                } else {
                    queryParts.push_back(decodedKey);
                }
            }
        }
        if (!queryParts.empty()) {
            canonicalQuery = "?" + utils::StringJoin(queryParts, "&");
        }
    }

    std::string canonicalResource = canonicalUri + canonicalQuery;

    std::string canonicalHeaders = buildCanonicalHeaders(request->headers);

    std::string contentMd5;
    std::string contentType;
    for (const auto& [k, v] : request->headers) {
        std::string lowerKey = utils::ToLower(k.c_str());
        if (lowerKey == "content-md5") {
            contentMd5 = v;
        } else if (lowerKey == "content-type") {
            contentType = v;
        }
    }

    std::string dateHeader = dateOverride.empty() ? getDateFromHeaders(request->headers) : dateOverride;

    std::stringstream ss;
    ss << request->method << "\n"
       << contentMd5 << "\n"
       << contentType << "\n"
       << dateHeader << "\n"
       << canonicalHeaders << canonicalResource;

    return ss.str();
}

static std::string calcSignature(const std::string& secret, const std::string& stringToSign) {
    unsigned char out[20];
    utils::HmacSha1(stringToSign.data(), stringToSign.size(),
                    secret.data(), secret.size(), out);
    return utils::Base64Encode(reinterpret_cast<const std::byte*>(out), 20);
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

    auto dateRfc2822 = utils::ToGmtTime(timeNow);
    request->headers.emplace("Date", dateRfc2822);

    if (!cred.getSessionToken().empty()) {
        request->headers.emplace("x-oss-security-token", cred.getSessionToken());
    }

    auto stringToSign = calcStringToSign(context);
    auto signature = calcSignature(cred.getAccessKeySecret(), stringToSign);

    std::string credentialHeader = "OSS " + cred.getAccessKeyId() + ":" + signature;
    request->headers.emplace("Authorization", credentialHeader);

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

    std::string expires = std::to_string(expirationTime);

    // Build query parameters map first (like Java does)
    // Note: ToEncodedParameters returns already-encoded values
    std::map<std::string, std::string> params;

    // Get existing query params from URI
    auto queryPos = request->uri.find("?");
    if (queryPos != std::string::npos) {
        auto existingParams = utils::ToEncodedParameters(request->uri.substr(queryPos));
        for (const auto& [k, v] : existingParams) {
            params[k] = v;  // Keep already-encoded values
        }
    }

    // Add auth params (these need to be encoded when building URI)
    params["OSSAccessKeyId"] = utils::UrlEncode(cred.getAccessKeyId());
    params["Expires"] = expires;

    if (!cred.getSessionToken().empty()) {
        params["security-token"] = utils::UrlEncode(cred.getSessionToken());
    }

    // Build new URI (without Signature yet)
    std::stringstream ss;
    ss << request->uri.substr(0, queryPos) << "?";
    bool first = true;
    for (const auto& [k, v] : params) {
        if (!first) ss << "&";
        ss << k;
        if (!v.empty()) ss << "=" << v;
        first = false;
    }

    // Temporarily update request URI for stringToSign calculation
    std::string originalUri = request->uri;
    request->uri = ss.str();

    // Calculate stringToSign and signature
    auto stringToSign = calcStringToSign(context, expires);
    auto signature = calcSignature(cred.getAccessKeySecret(), stringToSign);

    // Add Signature to URI
    request->uri = ss.str() + "&Signature=" + utils::UrlEncode(signature);

    context.stringToSign = std::move(stringToSign);
    context.signTimeInEpoch = timeNow;
    context.expirationInEpoch = expirationTime;
}

} // namespace signer::v1


bool SignerV1::sign(SigningContext& context) {
    if (context.request == nullptr || !context.credentials.hasKeys()) {
        return false;
    }

    if (context.authMethodQuery) {
        signer::v1::authQuery(context);
    } else {
        signer::v1::authHeader(context);
    }
    return true;
}


} // namespace alibabacloud::oss2
