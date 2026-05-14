#include "SerdeService.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static models::BucketSummary toBucket(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::BucketSummary();

    node = root->FirstChildElement("Name");

    if (node) {
        result.name = toString(node);
    }


    node = root->FirstChildElement("StorageClass");

    if (node) {
        result.storageClass = toString(node);
    }


    node = root->FirstChildElement("Region");

    if (node) {
        result.region = toString(node);
    }


    node = root->FirstChildElement("CreationDate");

    if (node) {
        result.creationDate = toString(node);
    }


    node = root->FirstChildElement("ExtranetEndpoint");

    if (node) {
        result.extranetEndpoint = toString(node);
    }


    node = root->FirstChildElement("IntranetEndpoint");

    if (node) {
        result.intranetEndpoint = toString(node);
    }


    node = root->FirstChildElement("Location");

    if (node) {
        result.location = toString(node);
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

inline static models::ListAllMyBucketsResult toListAllMyBucketsResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ListAllMyBucketsResult();

    node = root->FirstChildElement("Marker");

    if (node) {
        result.marker = toString(node);
    }


    node = root->FirstChildElement("MaxKeys");

    if (node) {
        result.maxKeys = toInt64(node);
    }


    node = root->FirstChildElement("IsTruncated");

    if (node) {
        result.isTruncated = toBool(node);
    }


    node = root->FirstChildElement("NextMarker");

    if (node) {
        result.nextMarker = toString(node);
    }

    node = root->FirstChildElement("Buckets");
    if (node) {
        auto subnode = node->FirstChildElement("Bucket");
        for (; subnode; subnode = subnode->NextSiblingElement("Bucket")) {
            result.buckets.emplace_back(toBucket(subnode));
        }
    }

    node = root->FirstChildElement("Owner");

    if (node) {
        result.owner = toOwner(node);
    }


    node = root->FirstChildElement("Prefix");

    if (node) {
        result.prefix = toString(node);
    }


    return result;
}


OperationInput fromListBuckets(const models::ListBucketsRequest& request) {
    auto input = OperationInput{"ListBuckets", "GET"};

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


    return input;
}

Outcome<models::ListBucketsResult, OperationError> toListBuckets(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListAllMyBucketsResult", root->Name())) {
                return models::ListBucketsResult(output.statusCode, std::move(output.headers),
                                                 toListAllMyBucketsResult(root));
            }
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
    return models::ListBucketsResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud