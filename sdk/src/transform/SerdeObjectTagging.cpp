#include "SerdeObjectTagging.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static std::string toXmlText(const models::Tag& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (!value.key.empty()) {
        str.append(toXmlText(value.key, "Key"));
    }

    if (!value.value.empty()) {
        str.append(toXmlText(value.value, "Value"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::Tag toTag(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::Tag();

    node = root->FirstChildElement("Key");

    if (node) {
        result.key = toString(node);
    }


    node = root->FirstChildElement("Value");

    if (node) {
        result.value = toString(node);
    }


    return result;
}


inline static std::string toXmlText(const models::TagSet& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    for (const auto& it : value.tags) {
        str.append(toXmlText(it, "Tag"));
    }

    str.append("</").append(tag).append(">");
    return str;
}


inline static models::TagSet toTagSet(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::TagSet();

    node = root->FirstChildElement("Tag");

    for (; node; node = node->NextSiblingElement("Tag")) {
        result.tags.emplace_back(toTag(node));
    }


    return result;
}


inline static std::string toXmlText(const models::Tagging& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.tagSet.has_value()) {
        str.append(toXmlText(value.tagSet.value(), "TagSet"));
    }

    str.append("</").append(tag).append(">");
    return str;
}


inline static models::Tagging toTagging(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::Tagging();

    node = root->FirstChildElement("TagSet");

    if (node) {
        result.tagSet = toTagSet(node);
    }


    return result;
}


OperationInput fromPutObjectTagging(const models::PutObjectTaggingRequest& request) {
    auto input = OperationInput{"PutObjectTagging", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("tagging", "");


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

    if (request.hasTagging()) {
        auto str = toXmlText(request.getTagging(), "Tagging");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::PutObjectTaggingResult, OperationError> toPutObjectTagging(OperationOutput&& output) {
    return models::PutObjectTaggingResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetObjectTagging(const models::GetObjectTaggingRequest& request) {
    auto input = OperationInput{"GetObjectTagging", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("tagging", "");


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

Outcome<models::GetObjectTaggingResult, OperationError> toGetObjectTagging(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetObjectTaggingResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("Tagging", root->Name())) {
                result.setTagging(toTagging(root));
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

    return models::GetObjectTaggingResult(output.statusCode, std::move(output.headers));
}


OperationInput fromDeleteObjectTagging(const models::DeleteObjectTaggingRequest& request) {
    auto input = OperationInput{"DeleteObjectTagging", "DELETE"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("tagging", "");


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

Outcome<models::DeleteObjectTaggingResult, OperationError> toDeleteObjectTagging(OperationOutput&& output) {
    return models::DeleteObjectTaggingResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud