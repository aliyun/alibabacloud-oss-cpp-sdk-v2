#include "SerdeAgenticBucket.h"
#include "src/transform/SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"

#include <cstring>
#include <istream>

namespace alibabacloud {
namespace oss2 {
namespace agentic {
namespace transform {

using oss2::transform::toBool;
using oss2::transform::toInt32;
using oss2::transform::toString;
using oss2::transform::toXmlText;

inline static std::string toXmlText(const models::CreateAgenticBucketConfiguration& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.storageClass.has_value()) {
        str.append(toXmlText(value.storageClass.value(), "StorageClass"));
    }
    if (value.dataRedundancyType.has_value()) {
        str.append(toXmlText(value.dataRedundancyType.value(), "DataRedundancyType"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::AgenticBucketStatus& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    str.append(toXmlText(value.status, "Status"));
    str.append("</").append(tag).append(">");
    return str;
}

inline static oss2::models::Owner toOwner(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = oss2::models::Owner();
    node = root->FirstChildElement("ID");
    if (node) {
        result.id = toString(node);
    }
    node = root->FirstChildElement("DisplayName");
    if (node) {
        result.displayName = toString(node);
    }
    return result;
}

inline static oss2::models::ServerSideEncryptionRule toServerSideEncryptionRule(
    const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = oss2::models::ServerSideEncryptionRule();
    // SSE fields are nested under ApplyServerSideEncryptionByDefault per spec;
    // fall back to the rule element itself for a flat shape.
    const auto* apply = root->FirstChildElement("ApplyServerSideEncryptionByDefault");
    const auto* base = apply ? apply : root;
    node = base->FirstChildElement("KMSMasterKeyID");
    if (node) {
        result.kmsMasterKeyID = toString(node);
    }
    node = base->FirstChildElement("KMSDataEncryption");
    if (node) {
        result.kmsDataEncryption = toString(node);
    }
    node = base->FirstChildElement("SSEAlgorithm");
    if (node) {
        result.sseAlgorithm = toString(node);
    }
    return result;
}

inline static models::AgenticBucketInfo toAgenticBucketInfo(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::AgenticBucketInfo();
    node = root->FirstChildElement("Name");
    if (node) {
        result.name = toString(node);
    }
    node = root->FirstChildElement("Owner");
    if (node) {
        result.owner = toString(node);
    }
    node = root->FirstChildElement("Region");
    if (node) {
        result.region = toString(node);
    }
    node = root->FirstChildElement("StorageClass");
    if (node) {
        result.storageClass = toString(node);
    }
    node = root->FirstChildElement("DataRedundancyType");
    if (node) {
        result.dataRedundancyType = toString(node);
    }
    node = root->FirstChildElement("Status");
    if (node) {
        result.status = toString(node);
    }
    node = root->FirstChildElement("BucketResourceType");
    if (node) {
        result.bucketResourceType = toString(node);
    }
    node = root->FirstChildElement("CreateTime");
    if (node) {
        result.createTime = toString(node);
    }
    node = root->FirstChildElement("ACL");
    if (node) {
        result.acl = toString(node);
    }
    node = root->FirstChildElement("PublicAccessBlock");
    if (node) {
        result.publicAccessBlock = toString(node);
    }
    node = root->FirstChildElement("ServerSideEncryptionRule");
    if (node) {
        result.serverSideEncryptionRule = toServerSideEncryptionRule(node);
    }
    node = root->FirstChildElement("Versioning");
    if (node) {
        result.versioning = toString(node);
    }
    node = root->FirstChildElement("BucketPolicy");
    if (node) {
        result.bucketPolicy = toString(node);
    }
    return result;
}

inline static models::AgenticBucketSummary toAgenticBucketSummary(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::AgenticBucketSummary();
    node = root->FirstChildElement("Name");
    if (node) {
        result.name = toString(node);
    }
    node = root->FirstChildElement("StorageClass");
    if (node) {
        result.storageClass = toString(node);
    }
    node = root->FirstChildElement("DataRedundancyType");
    if (node) {
        result.dataRedundancyType = toString(node);
    }
    node = root->FirstChildElement("CreateTime");
    if (node) {
        result.createTime = toString(node);
    }
    return result;
}

inline static models::BucketSpaceSummary toBucketSpaceSummary(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::BucketSpaceSummary();
    node = root->FirstChildElement("Name");
    if (node) {
        result.name = toString(node);
    }
    node = root->FirstChildElement("Location");
    if (node) {
        result.location = toString(node);
    }
    node = root->FirstChildElement("CreationDate");
    if (node) {
        result.creationDate = toString(node);
    }
    node = root->FirstChildElement("StorageClass");
    if (node) {
        result.storageClass = toString(node);
    }
    return result;
}

inline static models::ListAgenticBucketsResult toListAgenticBucketsResult(
    int statusCode, HeaderCollection&& headers, const thirdparty::tinyxml2::XMLElement* root) {
    auto result = models::ListAgenticBucketsResult(statusCode, std::move(headers));
    const thirdparty::tinyxml2::XMLElement* node;
    node = root->FirstChildElement("Region");
    if (node) {
        result.setRegion(toString(node));
    }
    node = root->FirstChildElement("Owner");
    if (node) {
        result.setOwner(toString(node));
    }
    node = root->FirstChildElement("ContinuationToken");
    if (node) {
        result.setContinuationToken(toString(node));
    }
    node = root->FirstChildElement("NextContinuationToken");
    if (node) {
        result.setNextContinuationToken(toString(node));
    }
    node = root->FirstChildElement("IsTruncated");
    if (node) {
        result.setIsTruncated(toBool(node));
    }
    std::vector<models::AgenticBucketSummary> buckets;
    const auto* wrapper = root->FirstChildElement("AgenticBuckets");
    if (wrapper) {
        for (const auto* item = wrapper->FirstChildElement("AgenticBucket"); item;
             item = item->NextSiblingElement("AgenticBucket")) {
            buckets.emplace_back(toAgenticBucketSummary(item));
        }
    }
    result.setAgenticBuckets(std::move(buckets));
    return result;
}

inline static models::ListBucketSpacesResult toListBucketSpacesResult(int statusCode, HeaderCollection&& headers,
                                                                     const thirdparty::tinyxml2::XMLElement* root) {
    auto result = models::ListBucketSpacesResult(statusCode, std::move(headers));
    const thirdparty::tinyxml2::XMLElement* node;
    node = root->FirstChildElement("Owner");
    if (node) {
        result.setOwner(toOwner(node));
    }
    node = root->FirstChildElement("Prefix");
    if (node) {
        result.setPrefix(toString(node));
    }
    node = root->FirstChildElement("MaxKeys");
    if (node) {
        result.setMaxKeys(toInt32(node));
    }
    node = root->FirstChildElement("ContinuationToken");
    if (node) {
        result.setContinuationToken(toString(node));
    }
    node = root->FirstChildElement("NextContinuationToken");
    if (node) {
        result.setNextContinuationToken(toString(node));
    }
    node = root->FirstChildElement("StartAfter");
    if (node) {
        result.setStartAfter(toString(node));
    }
    node = root->FirstChildElement("IsTruncated");
    if (node) {
        result.setIsTruncated(toBool(node));
    }
    std::vector<models::BucketSpaceSummary> spaces;
    const auto* wrapper = root->FirstChildElement("BucketSpaces");
    if (wrapper) {
        for (const auto* item = wrapper->FirstChildElement("BucketSpace"); item;
             item = item->NextSiblingElement("BucketSpace")) {
            spaces.emplace_back(toBucketSpaceSummary(item));
        }
    }
    result.setBucketSpaces(std::move(spaces));
    return result;
}

OperationInput fromCreateAgenticBucket(const models::CreateAgenticBucketRequest& request) {
    auto input = OperationInput{"CreateAgenticBucket", "PUT"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";
    if (request.hasCreateAgenticBucketConfiguration()) {
        auto str = toXmlText(request.getCreateAgenticBucketConfiguration(), "CreateAgenticBucketConfiguration");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }
    input.headers.emplace("Content-MD5", std::move(md5));
    input.bucket = request.getBucket();
    return input;
}

Outcome<models::CreateAgenticBucketResult, OperationError> toCreateAgenticBucket(OperationOutput&& output) {
    return models::CreateAgenticBucketResult(output.statusCode, std::move(output.headers));
}

OperationInput fromDeleteAgenticBucket(const models::DeleteAgenticBucketRequest& request) {
    auto input = OperationInput{"DeleteAgenticBucket", "DELETE"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    input.headers.emplace("Content-MD5", "1B2M2Y8AsgTpgAmY7PhCfg==");
    input.bucket = request.getBucket();
    return input;
}

Outcome<models::DeleteAgenticBucketResult, OperationError> toDeleteAgenticBucket(OperationOutput&& output) {
    return models::DeleteAgenticBucketResult(output.statusCode, std::move(output.headers));
}

OperationInput fromGetAgenticBucket(const models::GetAgenticBucketRequest& request) {
    auto input = OperationInput{"GetAgenticBucket", "GET"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    input.headers.emplace("Content-MD5", "1B2M2Y8AsgTpgAmY7PhCfg==");
    input.bucket = request.getBucket();
    return input;
}

Outcome<models::GetAgenticBucketResult, OperationError> toGetAgenticBucket(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetAgenticBucketResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("AgenticBucketInfo", root->Name())) {
                result.setAgenticBucketInfo(toAgenticBucketInfo(root));
            }
            return result;
        } else {
            auto opErr = OperationError{SerdeErrorCode::DeserializationFailed,
                                        {
                                            {"Code", "XMLError:" + std::to_string(static_cast<int>(xml_err))},
                                            {"Message", doc.ErrorStr()},
                                        }};
            opErr.setResponseResult(output.statusCode, std::move(output.headers), std::move(str));
            return makeUnexpected(std::move(opErr));
        }
    }
    return models::GetAgenticBucketResult(output.statusCode, std::move(output.headers));
}

OperationInput fromListAgenticBuckets(const models::ListAgenticBucketsRequest& request) {
    auto input = OperationInput{"ListAgenticBuckets", "GET"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    input.headers.emplace("Content-MD5", "1B2M2Y8AsgTpgAmY7PhCfg==");
    return input;
}

Outcome<models::ListAgenticBucketsResult, OperationError> toListAgenticBuckets(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListAgenticBucketsResult", root->Name())) {
                return toListAgenticBucketsResult(output.statusCode, std::move(output.headers), root);
            }
            return models::ListAgenticBucketsResult(output.statusCode, std::move(output.headers));
        } else {
            auto opErr = OperationError{SerdeErrorCode::DeserializationFailed,
                                        {
                                            {"Code", "XMLError:" + std::to_string(static_cast<int>(xml_err))},
                                            {"Message", doc.ErrorStr()},
                                        }};
            opErr.setResponseResult(output.statusCode, std::move(output.headers), std::move(str));
            return makeUnexpected(std::move(opErr));
        }
    }
    return models::ListAgenticBucketsResult(output.statusCode, std::move(output.headers));
}

OperationInput fromPutAgenticBucketStatus(const models::PutAgenticBucketStatusRequest& request) {
    auto input = OperationInput{"PutAgenticBucketStatus", "PUT"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    input.parameters.emplace("status", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";
    if (request.hasAgenticBucketStatus()) {
        auto str = toXmlText(request.getAgenticBucketStatus(), "AgenticBucketStatus");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }
    input.headers.emplace("Content-MD5", std::move(md5));
    input.bucket = request.getBucket();
    return input;
}

Outcome<models::PutAgenticBucketStatusResult, OperationError> toPutAgenticBucketStatus(OperationOutput&& output) {
    return models::PutAgenticBucketStatusResult(output.statusCode, std::move(output.headers));
}

OperationInput fromListBucketSpaces(const models::ListBucketSpacesRequest& request) {
    auto input = OperationInput{"ListBucketSpaces", "GET"};
    input.headers.emplace("Content-Type", "application/xml");
    input.parameters.emplace("agenticBucket", "");
    input.parameters.emplace("bucketSpace", "");
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }
    input.headers.emplace("Content-MD5", "1B2M2Y8AsgTpgAmY7PhCfg==");
    input.bucket = request.getBucket();
    return input;
}

Outcome<models::ListBucketSpacesResult, OperationError> toListBucketSpaces(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListBucketSpacesResult", root->Name())) {
                return toListBucketSpacesResult(output.statusCode, std::move(output.headers), root);
            }
            return models::ListBucketSpacesResult(output.statusCode, std::move(output.headers));
        } else {
            auto opErr = OperationError{SerdeErrorCode::DeserializationFailed,
                                        {
                                            {"Code", "XMLError:" + std::to_string(static_cast<int>(xml_err))},
                                            {"Message", doc.ErrorStr()},
                                        }};
            opErr.setResponseResult(output.statusCode, std::move(output.headers), std::move(str));
            return makeUnexpected(std::move(opErr));
        }
    }
    return models::ListBucketSpacesResult(output.statusCode, std::move(output.headers));
}

} // namespace transform
} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
