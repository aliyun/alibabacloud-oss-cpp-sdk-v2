#include "SerdeObjectMultipart.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static models::CopyPartResult toCopyPartResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::CopyPartResult();

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


inline static models::Upload toUpload(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::Upload();

    node = root->FirstChildElement("Key");

    if (node) {
        result.key = toString(node);
    }


    node = root->FirstChildElement("UploadId");

    if (node) {
        result.uploadId = toString(node);
    }


    node = root->FirstChildElement("Initiated");

    if (node) {
        result.initiated = toString(node);
    }


    return result;
}

/*
inline static models::CommonPrefix toCommonPrefix(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::CommonPrefix();

    node = root->FirstChildElement("Prefix");

    if (node) {
        result.prefix = toString(node);
    }


    return result;
}
*/

inline static std::string toXmlText(const models::Part& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (!value.eTag.empty()) {
        str.append(toXmlText(value.eTag, "ETag"));
    }

    if (value.partNumber > 0) {
        str.append(toXmlText(value.partNumber, "PartNumber"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::Part toPart(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::Part();

    node = root->FirstChildElement("ETag");

    if (node) {
        result.eTag = toString(node);
    }


    node = root->FirstChildElement("PartNumber");

    if (node) {
        result.partNumber = toInt64(node);
    }


    node = root->FirstChildElement("Size");

    if (node) {
        result.size = toInt64(node);
    }


    node = root->FirstChildElement("LastModified");

    if (node) {
        result.lastModified = toString(node);
    }


    return result;
}


inline static std::string toXmlText(const models::CompleteMultipartUpload& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    for (const auto& it : value.parts) {
        str.append(toXmlText(it, "Part"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::ListPartResultXml toListPartResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ListPartResultXml();

    node = root->FirstChildElement("EncodingType");
    if (node) {
        result.encodingType = toString(node);
    }
    bool doDecode = result.encodingType == "url";

    node = root->FirstChildElement("Key");

    if (node) {
        result.key = toString(node, doDecode);
    }

    node = root->FirstChildElement("UploadId");

    if (node) {
        result.uploadId = toString(node);
    }


    node = root->FirstChildElement("PartNumberMarker");

    if (node) {
        result.partNumberMarker = toInt64(node);
    }


    node = root->FirstChildElement("NextPartNumberMarker");

    if (node) {
        result.nextPartNumberMarker = toInt64(node);
    }


    node = root->FirstChildElement("MaxParts");

    if (node) {
        result.maxParts = toInt64(node);
    }


    node = root->FirstChildElement("IsTruncated");

    if (node) {
        result.isTruncated = toBool(node);
    }


    node = root->FirstChildElement("Part");

    for (; node; node = node->NextSiblingElement("Part")) {
        result.parts.emplace_back(toPart(node));
    }


    node = root->FirstChildElement("Bucket");

    if (node) {
        result.bucket = toString(node);
    }


    return result;
}

inline static models::InitiateMultipartUploadResultXml toInitiateMultipartUploadResultXml(
        const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::InitiateMultipartUploadResultXml();

    node = root->FirstChildElement("EncodingType");
    if (node) {
        result.encodingType = toString(node);
    }
    bool doDecode = result.encodingType == "url";

    node = root->FirstChildElement("Bucket");
    if (node) {
        result.bucket = toString(node);
    }

    node = root->FirstChildElement("Key");
    if (node) {
        result.key = toString(node, doDecode);
    }

    node = root->FirstChildElement("UploadId");
    if (node) {
        result.uploadId = toString(node);
    }
    return result;
}


inline static models::ListMultipartUploadsResultXml toListMultipartUploadsResult(
        const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ListMultipartUploadsResultXml();

    node = root->FirstChildElement("EncodingType");
    if (node) {
        result.encodingType = toString(node);
    }
    bool doDecode = result.encodingType == "url";

    node = root->FirstChildElement("Bucket");
    if (node) {
        result.bucket = toString(node);
    }

    node = root->FirstChildElement("KeyMarker");
    if (node) {
        result.keyMarker = toString(node, doDecode);
    }

    node = root->FirstChildElement("UploadIdMarker");
    if (node) {
        result.uploadIdMarker = toString(node);
    }

    node = root->FirstChildElement("NextKeyMarker");
    if (node) {
        result.nextKeyMarker = toString(node, doDecode);
    }

    node = root->FirstChildElement("NextUploadIdMarker");
    if (node) {
        result.nextUploadIdMarker = toString(node);
    }

    node = root->FirstChildElement("Delimiter");
    if (node) {
        result.delimiter = toString(node, doDecode);
    }

    node = root->FirstChildElement("Prefix");
    if (node) {
        result.prefix = toString(node, doDecode);
    }

    node = root->FirstChildElement("MaxUploads");
    if (node) {
        result.maxUploads = toInt64(node);
    }

    node = root->FirstChildElement("IsTruncated");
    if (node) {
        result.isTruncated = toBool(node);
    }

    node = root->FirstChildElement("Upload");
    for (; node; node = node->NextSiblingElement("Upload")) {
        result.uploads.emplace_back(toUpload(node));
        if (doDecode) {
            auto& item = result.uploads.back();
            item.key = utils::UrlDecode(item.key);
        }
    }

    return result;
}

OperationInput fromInitiateMultipartUpload(const models::InitiateMultipartUploadRequest& request) {
    auto input = OperationInput{"InitiateMultipartUpload", "POST"};

    // Default Headers
    //input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("uploads", "");
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
    input.key = request.getKey();

    return input;
}

Outcome<models::InitiateMultipartUploadResult, OperationError> toInitiateMultipartUpload(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("InitiateMultipartUploadResult", root->Name())) {
                return models::InitiateMultipartUploadResult(output.statusCode, std::move(output.headers),
                                                             toInitiateMultipartUploadResultXml(root));
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

    return models::InitiateMultipartUploadResult(output.statusCode, std::move(output.headers));
}


OperationInput fromUploadPart(const models::UploadPartRequest& request) {
    auto input = OperationInput{"UploadPart", "PUT"};

    // Default Headers
    // input.headers.emplace("Content-Type", "application/xml");

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
    input.body = request.getBody();
    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::UploadPartResult, OperationError> toUploadPart(OperationOutput&& output) {
    return models::UploadPartResult(output.statusCode, std::move(output.headers));
}


OperationInput fromCompleteMultipartUpload(const models::CompleteMultipartUploadRequest& request) {
    auto input = OperationInput{"CompleteMultipartUpload", "POST"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
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

    if (request.hasCompleteMultipartUpload()) {
        auto str = toXmlText(request.getCompleteMultipartUpload(), "CompleteMultipartUpload");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::FromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::CompleteMultipartUploadResult, OperationError> toCompleteMultipartUpload(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root == nullptr) {
                auto opErr = OperationError{SerdeErrorCode::DeserializationFailed,
                                            {
                                                    {"Code", "XMLError"},
                                                    {"Message", "RootElement is null"},
                                            }};
                return makeUnexpected(std::move(opErr));
            }
            auto result = models::CompleteMultipartUploadResult(output.statusCode, std::move(output.headers));
            auto node = root->FirstChildElement("EncodingType");
            bool doDecode = false;
            if (node) {
                auto value = toString(node);
                doDecode = value == "url";
                result.setEncodingType(value);
            }

            node = root->FirstChildElement("Location");
            if (node) {
                result.setLocation(toString(node));
            }

            node = root->FirstChildElement("Bucket");
            if (node) {
                result.setBucket(toString(node));
            }

            node = root->FirstChildElement("Key");
            if (node) {
                result.setKey(toString(node, doDecode));
            }

            node = root->FirstChildElement("ETag");
            if (node) {
                result.setETag(toString(node));
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

    return models::CompleteMultipartUploadResult(output.statusCode, std::move(output.headers));
}


OperationInput fromUploadPartCopy(const models::UploadPartCopyRequest& request) {
    auto input = OperationInput{"UploadPartCopy", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters


    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    if (!request.getSourceKey().empty()) {
        std::string source;
        source.append("/");
        source.append(request.getSourceBucket().empty()?request.getBucket():request.getSourceBucket());
        source.append("/").append(utils::UrlEncode(request.getSourceKey()));
        if (!request.getSourceVersionId().empty()) {
            source.append("?versionId=").append(request.getSourceVersionId());
        }
        input.headers.insert_or_assign("x-oss-copy-source",  source);
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

Outcome<models::UploadPartCopyResult, OperationError> toUploadPartCopy(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("CopyPartResult", root->Name())) {
                return models::UploadPartCopyResult(output.statusCode, std::move(output.headers),
                                                    toCopyPartResult(root));
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

    return models::UploadPartCopyResult(output.statusCode, std::move(output.headers));
}


OperationInput fromAbortMultipartUpload(const models::AbortMultipartUploadRequest& request) {
    auto input = OperationInput{"AbortMultipartUpload", "DELETE"};

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

Outcome<models::AbortMultipartUploadResult, OperationError> toAbortMultipartUpload(OperationOutput&& output) {
    return models::AbortMultipartUploadResult(output.statusCode, std::move(output.headers));
}


OperationInput fromListMultipartUploads(const models::ListMultipartUploadsRequest& request) {
    auto input = OperationInput{"ListMultipartUploads", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("uploads", "");
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

Outcome<models::ListMultipartUploadsResult, OperationError> toListMultipartUploads(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListMultipartUploadsResult", root->Name())) {
                return models::ListMultipartUploadsResult(output.statusCode, std::move(output.headers),
                                                          toListMultipartUploadsResult(root));
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

    return models::ListMultipartUploadsResult(output.statusCode, std::move(output.headers));
}


OperationInput fromListParts(const models::ListPartsRequest& request) {
    auto input = OperationInput{"ListParts", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
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
    input.key = request.getKey();

    return input;
}

Outcome<models::ListPartsResult, OperationError> toListParts(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListPartsResult", root->Name())) {
                return models::ListPartsResult(output.statusCode, std::move(output.headers), toListPartResult(root));
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

    return models::ListPartsResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud