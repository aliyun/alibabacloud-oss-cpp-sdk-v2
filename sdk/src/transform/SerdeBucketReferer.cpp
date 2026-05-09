#include "SerdeBucketReferer.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static std::string toXmlText(const models::RefererList& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    for (const auto& it : value.referers) {
        str.append(toXmlText(it, "Referer"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::RefererList toRefererList(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::RefererList();

    node = root->FirstChildElement("Referer");

    for (; node; node = node->NextSiblingElement("Referer")) {
        result.referers.emplace_back(toString(node));
    }


    return result;
}


inline static std::string toXmlText(const models::RefererBlacklist& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    for (const auto& it : value.referers) {
        str.append(toXmlText(it, "Referer"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::RefererBlacklist toRefererBlacklist(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::RefererBlacklist();

    node = root->FirstChildElement("Referer");

    for (; node; node = node->NextSiblingElement("Referer")) {
        result.referers.emplace_back(toString(node));
    }


    return result;
}


inline static std::string toXmlText(const models::RefererConfiguration& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");

    if (value.allowEmptyReferer.has_value()) {
        str.append(toXmlText(value.allowEmptyReferer.value(), "AllowEmptyReferer"));
    }

    if (value.allowTruncateQueryString.has_value()) {
        str.append(toXmlText(value.allowTruncateQueryString.value(), "AllowTruncateQueryString"));
    }

    if (value.truncatePath.has_value()) {
        str.append(toXmlText(value.truncatePath.value(), "TruncatePath"));
    }

    if (value.refererList.has_value()) {
        str.append(toXmlText(value.refererList.value(), "RefererList"));
    }

    if (value.refererBlacklist.has_value()) {
        str.append(toXmlText(value.refererBlacklist.value(), "RefererBlacklist"));
    }

    str.append("</").append(tag).append(">");
    return str;
}

inline static models::RefererConfiguration toRefererConfiguration(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::RefererConfiguration();

    node = root->FirstChildElement("AllowEmptyReferer");

    if (node) {
        result.allowEmptyReferer = toBool(node);
    }


    node = root->FirstChildElement("AllowTruncateQueryString");

    if (node) {
        result.allowTruncateQueryString = toBool(node);
    }


    node = root->FirstChildElement("TruncatePath");

    if (node) {
        result.truncatePath = toBool(node);
    }


    node = root->FirstChildElement("RefererList");

    if (node) {
        result.refererList = toRefererList(node);
    }


    node = root->FirstChildElement("RefererBlacklist");

    if (node) {
        result.refererBlacklist = toRefererBlacklist(node);
    }


    return result;
}


OperationInput fromPutBucketReferer(const models::PutBucketRefererRequest& request) {
    auto input = OperationInput{"PutBucketReferer", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("referer", "");


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

    if (request.hasRefererConfiguration()) {
        auto str = toXmlText(request.getRefererConfiguration(), "RefererConfiguration");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::FromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();


    return input;
}

Outcome<models::PutBucketRefererResult, OperationError> toPutBucketReferer(OperationOutput&& output) {
    return models::PutBucketRefererResult(output.statusCode, std::move(output.headers));
}

OperationInput fromGetBucketReferer(const models::GetBucketRefererRequest& request) {
    auto input = OperationInput{"GetBucketReferer", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("referer", "");


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

Outcome<models::GetBucketRefererResult, OperationError> toGetBucketReferer(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetBucketRefererResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("RefererConfiguration", root->Name())) {
                result.setRefererConfiguration(toRefererConfiguration(root));
            }
            return result;
        } else {
            auto opErr = OperationError{SdkErrorCode::Deserialization_ERROR,
                                        {
                                                {"Code", "XMLError:" + std::to_string(static_cast<int>(xml_err))},
                                                {"Message", doc.ErrorStr()},
                                        }};
            opErr.setResponseResult(output.statusCode, std::move(output.headers), std::move(str));
            return opErr;
        }
    }

    return models::GetBucketRefererResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud