#include "alibabacloud/oss2/signer/SignerV1.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/utils/Utils.h"


#include <set>

namespace alibabacloud::oss2 {


namespace singer::v1 {

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

const static std::set<std::string> ParamtersToSign = {"acl",
                                                      "location",
                                                      "bucketInfo",
                                                      "stat",
                                                      "referer",
                                                      "cors",
                                                      "website",
                                                      "restore",
                                                      "logging",
                                                      "symlink",
                                                      "qos",
                                                      "uploadId",
                                                      "uploads",
                                                      "partNumber",
                                                      "response-content-type",
                                                      "response-content-language",
                                                      "response-expires",
                                                      "response-cache-control",
                                                      "response-content-disposition",
                                                      "response-content-encoding",
                                                      "append",
                                                      "position",
                                                      "lifecycle",
                                                      "delete",
                                                      "live",
                                                      "status",
                                                      "comp",
                                                      "vod",
                                                      "startTime",
                                                      "endTime",
                                                      "x-oss-process",
                                                      "security-token",
                                                      "objectMeta",
                                                      "callback",
                                                      "callback-var",
                                                      "tagging",
                                                      "policy",
                                                      "requestPayment",
                                                      "x-oss-traffic-limit",
                                                      "encryption",
                                                      "qosInfo",
                                                      "versioning",
                                                      "versionId",
                                                      "versions",
                                                      "x-oss-request-payer",
                                                      "sequential",
                                                      "inventory",
                                                      "inventoryId",
                                                      "continuation-token",
                                                      "worm",
                                                      "wormId",
                                                      "wormExtend",
                                                      "regionList"};

static std::string calcStringToSign(SigningContext& context) {
    ((void) (context));
    return "";
}

static std::string calcSignature(const std::string& secrect, const std::string& stringToSign) {
    ((void) (secrect));
    ((void) (stringToSign));
    return "";
}

static void authHeader(SigningContext& context) {
    RequestMessage* request = context.request;
    Credentials& cred = context.credentials;

    std::time_t timeNow;
    if (context.signTimeInEpoch > 0) {
        timeNow = context.signTimeInEpoch;
    } else {
        timeNow = std::time(nullptr) + context.clockOffset.count();
    }

    auto dateRfc2822 = utils::ToGmtTime(timeNow);
    request->headers.emplace("Date", dateRfc2822);

    if (!cred.getSessionToken().empty()) {
        context.request->headers.emplace("x-oss-security-token", cred.getSessionToken());
    }

    auto stringToSign = calcStringToSign(context);
    auto signature = calcSignature(cred.getAccessKeySecret(), stringToSign);

    std::string credentialHeader = "OSS " + cred.getAccessKeyId() + ":" + signature;
    request->headers.emplace("Authorization", credentialHeader);

    context.stringToSign = std::move(stringToSign);
    context.signTimeInEpoch = timeNow;
}

static void authQuery(SigningContext& context) {
    // RequestMessage* request = context.request;
    Credentials& cred = context.credentials;

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

    // request->parameters.erase("Signature");
    // request->parameters.erase("security-token");

    // request->parameters.emplace("OSSAccessKeyId", cred.getAccessKeyId());
    // request->parameters.emplace("Expires", std::to_string(expirationTime));

    if (!cred.getSessionToken().empty()) {
        context.request->headers.emplace("security-token", cred.getSessionToken());
    }

    auto stringToSign = calcStringToSign(context);
    auto signature = calcSignature(cred.getAccessKeySecret(), stringToSign);

    // request->parameters.emplace("Signature", std::move(signature));

    context.stringToSign = std::move(stringToSign);
    context.signTimeInEpoch = timeNow;
    context.expirationInEpoch = expirationTime;
}

} // namespace singer::v1


bool SignerV1::sign(SigningContext& context) {
    if (context.request == nullptr || !context.credentials.hasKeys()) {
        return false;
    }

    if (context.authMethodQuery) {
        singer::v1::authQuery(context);
    } else {
        singer::v1::authHeader(context);
    }
    return true;
}


} // namespace alibabacloud::oss2
