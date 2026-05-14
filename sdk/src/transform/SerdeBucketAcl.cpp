#include "SerdeBucketAcl.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {

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

inline static models::AccessControlList toAccessControlList(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::AccessControlList();

    node = root->FirstChildElement("Grant");

    if (node) {
        result.grant = toString(node);
    }


    return result;
}

inline static models::AccessControlPolicy toAccessControlPolicy(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::AccessControlPolicy();

    node = root->FirstChildElement("Owner");

    if (node) {
        result.owner = toOwner(node);
    }


    node = root->FirstChildElement("AccessControlList");

    if (node) {
        result.accessControlList = toAccessControlList(node);
    }


    return result;
}


OperationInput fromPutBucketAcl(const models::PutBucketAclRequest& request) {
    auto input = OperationInput{"PutBucketAcl", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("acl", "");


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

Outcome<models::PutBucketAclResult, OperationError> toPutBucketAcl(OperationOutput&& output) {
    return models::PutBucketAclResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetBucketAcl(const models::GetBucketAclRequest& request) {
    auto input = OperationInput{"GetBucketAcl", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("acl", "");


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

Outcome<models::GetBucketAclResult, OperationError> toGetBucketAcl(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetBucketAclResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("AccessControlPolicy", root->Name())) {
                result.setAccessControlPolicy(toAccessControlPolicy(root));
            }
            return result;
        } else {
            auto opErr = OperationError{SdkErrorCode::Deserialization_ERROR,
                                        {
                                                {"Code", "XMLError:" + std::to_string(static_cast<int>(xml_err))},
                                                {"Message", doc.ErrorStr()},
                                        }};
            opErr.setResponseResult(output.statusCode, std::move(output.headers), std::move(str));
            return makeUnexpected(std::move(opErr));
        }
    }

    return models::GetBucketAclResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud