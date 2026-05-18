#include "SerdeBucketVersioning.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static std::string toXmlText(const models::VersioningConfiguration& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.status.has_value()) {
        str.append(toXmlText(value.status.value(), "Status"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::VersioningConfiguration toVersioningConfiguration(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::VersioningConfiguration();

    node = root->FirstChildElement("Status");

    if (node) {
        result.status = toString(node);
    }

    return result;
}

inline static models::Owner toOwner(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::Owner();

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

inline static models::ObjectVersion toObjectVersion(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ObjectVersion();

    node = root->FirstChildElement("Key");
    if (node) {
        result.key = toString(node);
    }

    node = root->FirstChildElement("VersionId");
    if (node) {
        result.versionId = toString(node);
    }

    node = root->FirstChildElement("IsLatest");
    if (node) {
        result.isLatest = toBool(node);
    }

    node = root->FirstChildElement("LastModified");
    if (node) {
        result.lastModified = toString(node);
    }

    node = root->FirstChildElement("ETag");
    if (node) {
        result.eTag = toString(node);
    }

    node = root->FirstChildElement("Size");
    if (node) {
        result.size = toInt64(node);
    }

    node = root->FirstChildElement("StorageClass");
    if (node) {
        result.storageClass = toString(node);
    }

    node = root->FirstChildElement("Owner");
    if (node) {
        result.owner = toOwner(node);
    }

    node = root->FirstChildElement("RestoreInfo");
    if (node) {
        result.restoreInfo = toString(node);
    }

    return result;
}

inline static models::DeleteMarkerEntry toDeleteMarkerEntry(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::DeleteMarkerEntry();

    node = root->FirstChildElement("Key");
    if (node) {
        result.key = toString(node);
    }

    node = root->FirstChildElement("VersionId");
    if (node) {
        result.versionId = toString(node);
    }

    node = root->FirstChildElement("IsLatest");
    if (node) {
        result.isLatest = toBool(node);
    }

    node = root->FirstChildElement("LastModified");
    if (node) {
        result.lastModified = toString(node);
    }

    node = root->FirstChildElement("Owner");
    if (node) {
        result.owner = toOwner(node);
    }

    return result;
}

inline static models::CommonPrefix toCommonPrefix(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::CommonPrefix();

    node = root->FirstChildElement("Prefix");
    if (node) {
        result.prefix = toString(node);
    }

    return result;
}

inline static models::ListVersionsResultXml toListVersionsResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ListVersionsResultXml();

    node = root->FirstChildElement("EncodingType");
    if (node) {
        result.encodingType = toString(node);
    }
    bool doDecode = result.encodingType == "url";

    node = root->FirstChildElement("Name");
    if (node) {
        result.name = toString(node);
    }

    node = root->FirstChildElement("Prefix");
    if (node) {
        result.prefix = toString(node, doDecode);
    }

    node = root->FirstChildElement("KeyMarker");
    if (node) {
        result.keyMarker = toString(node, doDecode);
    }

    node = root->FirstChildElement("VersionIdMarker");
    if (node) {
        result.versionIdMarker = toString(node);
    }

    node = root->FirstChildElement("NextKeyMarker");
    if (node) {
        result.nextKeyMarker = toString(node, doDecode);
    }

    node = root->FirstChildElement("NextVersionIdMarker");
    if (node) {
        result.nextVersionIdMarker = toString(node);
    }

    node = root->FirstChildElement("MaxKeys");
    if (node) {
        result.maxKeys = toInt32(node);
    }

    node = root->FirstChildElement("Delimiter");
    if (node) {
        result.delimiter = toString(node, doDecode);
    }

    node = root->FirstChildElement("IsTruncated");
    if (node) {
        result.isTruncated = toBool(node);
    }

    node = root->FirstChildElement("Version");
    for (; node; node = node->NextSiblingElement("Version")) {
        result.versions.emplace_back(toObjectVersion(node));
        if (doDecode) {
            auto& item = result.versions.back();
            item.key = utils::UrlDecode(item.key);
        }
    }

    node = root->FirstChildElement("DeleteMarker");
    for (; node; node = node->NextSiblingElement("DeleteMarker")) {
        result.deleteMarkers.emplace_back(toDeleteMarkerEntry(node));
        if (doDecode) {
            auto& item = result.deleteMarkers.back();
            item.key = utils::UrlDecode(item.key);
        }
    }

    node = root->FirstChildElement("CommonPrefixes");
    for (; node; node = node->NextSiblingElement("CommonPrefixes")) {
        result.commonPrefixes.emplace_back(toCommonPrefix(node));
        if (doDecode) {
            auto& item = result.commonPrefixes.back();
            item.prefix = utils::UrlDecode(item.prefix);
        }
    }

    return result;
}


OperationInput fromPutBucketVersioning(const models::PutBucketVersioningRequest& request) {
    auto input = OperationInput{"PutBucketVersioning", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("versioning", "");

    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    if (request.hasVersioningConfiguration()) {
        auto str = toXmlText(request.getVersioningConfiguration(), "VersioningConfiguration");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::FromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();


    return input;
}

Outcome<models::PutBucketVersioningResult, OperationError> toPutBucketVersioning(OperationOutput&& output) {
    return models::PutBucketVersioningResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetBucketVersioning(const models::GetBucketVersioningRequest& request) {
    auto input = OperationInput{"GetBucketVersioning", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("versioning", "");

    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();


    return input;
}

Outcome<models::GetBucketVersioningResult, OperationError> toGetBucketVersioning(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetBucketVersioningResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("VersioningConfiguration", root->Name())) {
                result.setVersioningConfiguration(toVersioningConfiguration(root));
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

    return models::GetBucketVersioningResult(output.statusCode, std::move(output.headers));
}


OperationInput fromListObjectVersions(const models::ListObjectVersionsRequest& request) {
    auto input = OperationInput{"ListObjectVersions", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("versions", "");
    input.parameters.emplace("encoding-type", "url");

    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();


    return input;
}

Outcome<models::ListObjectVersionsResult, OperationError> toListObjectVersions(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListVersionsResult", root->Name())) {
                return models::ListObjectVersionsResult(output.statusCode, std::move(output.headers),
                                                        toListVersionsResult(root));
            }
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

    return models::ListObjectVersionsResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud
