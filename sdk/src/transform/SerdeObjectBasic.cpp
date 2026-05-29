#include "SerdeObjectBasic.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static std::string toXmlText(const models::JobParameters& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.tier.has_value()) {
        str.append(toXmlText(value.tier.value(), "Tier"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::RestoreRequest& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.days.has_value()) {
        str.append(toXmlText(value.days.value(), "Days"));
    }

    if (value.jobParameters.has_value()) {
        str.append(toXmlText(value.jobParameters.value(), "JobParameters"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::Delete& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.quiet.has_value()) {
        str.append(toXmlText(value.quiet.value(), "Quiet"));
    }

    for (const auto& item : value.objects) {
        str.append("<Object>");
        str.append("<Key>").append(utils::XmlEscape(item.key)).append("</Key>");
        if (item.versionId.has_value()) {
            str.append("<VersionId>").append(utils::XmlEscape(item.versionId.value())).append("</VersionId>");
        }
        str.append("</Object>");
    }

    str.append("</").append(tag).append(">");
    return str;
}


inline static models::CopyObjectResultXml toCopyObjectResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::CopyObjectResultXml();

    node = root->FirstChildElement("LastModified");

    if (node) {
        result.lastModified = toString(node);
    }

    node = root->FirstChildElement("ETag");

    if (node) {
        result.eTag = toString(node);
    }


    return result;
}

inline static models::DeletedInfo toDeletedInfo(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::DeletedInfo();

    node = root->FirstChildElement("Key");
    if (node) {
        result.key = toString(node);
    }

    node = root->FirstChildElement("VersionId");
    if (node) {
        result.versionId = toString(node);
    }

    node = root->FirstChildElement("DeleteMarker");
    if (node) {
        result.deleteMarker = toBool(node);
    }

    node = root->FirstChildElement("DeleteMarkerVersionId");
    if (node) {
        result.deleteMarkerVersionId = toString(node);
    }

    return result;
}

OperationInput fromPutObject(const models::PutObjectRequest& request) {
    auto input = OperationInput{"PutObject", "PUT"};

    // Default Headers
    // TODO from key pattern
    // input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    for (const auto& [k, v] : request.getMetadata()) {
        input.headers.insert_or_assign("x-oss-meta-" + k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    input.body = request.getBody();
    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::PutObjectResult, OperationError> toPutObject(OperationOutput&& output) {
    auto result = models::PutObjectResult(output.statusCode, std::move(output.headers));
    if (output.body) {
        std::istreambuf_iterator<char> isb(*output.body), end;
        std::string body(isb, end);
        if (!body.empty()) {
            result.setCallbackResult(std::move(body));
        }
    }
    return result;
}


OperationInput fromCopyObject(const models::CopyObjectRequest& request) {
    auto input = OperationInput{"CopyObject", "PUT"};

    // Default Headers
    // TODO from key pattern
    // input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }
    for (const auto& [k, v] : request.getMetadata()) {
        input.headers.insert_or_assign("x-oss-meta-" + k, v);
    }

    if (!request.getSourceKey().empty()) {
        std::string source;
        source.append("/");
        source.append(request.getSourceBucket().empty() ? request.getBucket() : request.getSourceBucket());
        source.append("/").append(utils::UrlEncode(request.getSourceKey()));
        if (!request.getSourceVersionId().empty()) {
            source.append("?versionId=").append(request.getSourceVersionId());
        }
        input.headers.insert_or_assign("x-oss-copy-source", source);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::CopyObjectResult, OperationError> toCopyObject(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("CopyObjectResult", root->Name())) {
                return models::CopyObjectResult(output.statusCode, std::move(output.headers), toCopyObjectResult(root));
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
    return models::CopyObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetObject(const models::GetObjectRequest& request) {
    auto input = OperationInput{"GetObject", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters


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
    input.key = request.getKey();

    return input;
}

Outcome<models::GetObjectResult, OperationError> toGetObject(OperationOutput&& output) {
    return models::GetObjectResult(output.statusCode, std::move(output.headers), std::move(output.body));
}

OperationInput fromAppendObject(const models::AppendObjectRequest& request) {
    auto input = OperationInput{"AppendObject", "POST"};

    // Default Headers
    // input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("append", "");


    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    for (const auto& [k, v] : request.getMetadata()) {
        input.headers.insert_or_assign("x-oss-meta-" + k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    input.body = request.getBody();

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::AppendObjectResult, OperationError> toAppendObject(OperationOutput&& output) {
    return models::AppendObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromSealAppendObject(const models::SealAppendObjectRequest& request) {
    auto input = OperationInput{"SealAppendObject", "POST"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("seal", "");


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
    input.key = request.getKey();

    return input;
}

Outcome<models::SealAppendObjectResult, OperationError> toSealAppendObject(OperationOutput&& output) {
    return models::SealAppendObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromDeleteObject(const models::DeleteObjectRequest& request) {
    auto input = OperationInput{"DeleteObject", "DELETE"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters


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
    input.key = request.getKey();

    return input;
}

Outcome<models::DeleteObjectResult, OperationError> toDeleteObject(OperationOutput&& output) {
    return models::DeleteObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromDeleteMultipleObjects(const models::DeleteMultipleObjectsRequest& request) {
    auto input = OperationInput{"DeleteMultipleObjects", "POST"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("delete", "");
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

    if (request.hasDelete()) {
        auto str = toXmlText(request.getDelete(), "Delete");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();

    return input;
}

Outcome<models::DeleteMultipleObjectsResult, OperationError> toDeleteMultipleObjects(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("DeleteResult", root->Name())) {
                auto result = models::DeleteMultipleObjectsResult(output.statusCode, std::move(output.headers));
                const thirdparty::tinyxml2::XMLElement* node;
                node = root->FirstChildElement("EncodingType");
                if (node) {
                    result.setEncodingType(toString(node));
                }
                bool doDecode = result.getEncodingType() == "url";
                std::vector<models::DeletedInfo> deletedobjects;
                node = root->FirstChildElement("Deleted");
                for (; node; node = node->NextSiblingElement("Deleted")) {
                    deletedobjects.emplace_back(toDeletedInfo(node));
                    if (doDecode) {
                        deletedobjects.back().key = utils::UrlDecode(deletedobjects.back().key);
                    }
                }
                result.setDeletedObjects(std::move(deletedobjects));
                return result;
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
    return models::DeleteMultipleObjectsResult(output.statusCode, std::move(output.headers));
}


OperationInput fromHeadObject(const models::HeadObjectRequest& request) {
    auto input = OperationInput{"HeadObject", "HEAD"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters


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
    input.key = request.getKey();

    return input;
}

Outcome<models::HeadObjectResult, OperationError> toHeadObject(OperationOutput&& output) {
    return models::HeadObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetObjectMeta(const models::GetObjectMetaRequest& request) {
    auto input = OperationInput{"GetObjectMeta", "HEAD"};

    // Default Headers
    // input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("objectMeta", "");


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
    input.key = request.getKey();

    return input;
}

Outcome<models::GetObjectMetaResult, OperationError> toGetObjectMeta(OperationOutput&& output) {
    return models::GetObjectMetaResult(output.statusCode, std::move(output.headers));
}


OperationInput fromRestoreObject(const models::RestoreObjectRequest& request) {
    auto input = OperationInput{"RestoreObject", "POST"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("restore", "");


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

    if (request.hasRestoreRequest()) {
        auto str = toXmlText(request.getRestoreRequest(), "RestoreRequest");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::RestoreObjectResult, OperationError> toRestoreObject(OperationOutput&& output) {
    return models::RestoreObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromCleanRestoredObject(const models::CleanRestoredObjectRequest& request) {
    auto input = OperationInput{"CleanRestoredObject", "POST"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("cleanRestoredObject", "");


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
    input.key = request.getKey();

    return input;
}

Outcome<models::CleanRestoredObjectResult, OperationError> toCleanRestoredObject(OperationOutput&& output) {
    return models::CleanRestoredObjectResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud