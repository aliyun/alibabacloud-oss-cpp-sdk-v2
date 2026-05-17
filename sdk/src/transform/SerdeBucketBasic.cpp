#include "SerdeBucketBasic.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {

inline static models::BucketStat toBucketStat(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::BucketStat();

    node = root->FirstChildElement("DeleteMarkerCount");

    if (node) {
        result.deleteMarkerCount = toInt64(node);
    }


    node = root->FirstChildElement("ColdArchiveStorage");

    if (node) {
        result.coldArchiveStorage = toInt64(node);
    }


    node = root->FirstChildElement("ColdArchiveRealStorage");

    if (node) {
        result.coldArchiveRealStorage = toInt64(node);
    }


    node = root->FirstChildElement("InfrequentMultipartPartStorage");

    if (node) {
        result.infrequentMultipartPartStorage = toInt64(node);
    }


    node = root->FirstChildElement("ColdArchiveMultipartPartStorage");

    if (node) {
        result.coldArchiveMultipartPartStorage = toInt64(node);
    }


    node = root->FirstChildElement("StandardObjectCount");

    if (node) {
        result.standardObjectCount = toInt64(node);
    }


    node = root->FirstChildElement("ArchiveStorage");

    if (node) {
        result.archiveStorage = toInt64(node);
    }


    node = root->FirstChildElement("ColdArchiveObjectCount");

    if (node) {
        result.coldArchiveObjectCount = toInt64(node);
    }


    node = root->FirstChildElement("InfrequentMultipartPartCount");

    if (node) {
        result.infrequentMultipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("StandardMultipartPartCount");

    if (node) {
        result.standardMultipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("DeepColdArchiveMultipartPartCount");

    if (node) {
        result.deepColdArchiveMultipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("MultipartPartCount");

    if (node) {
        result.multipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("InfrequentAccessStorage");

    if (node) {
        result.infrequentAccessStorage = toInt64(node);
    }


    node = root->FirstChildElement("InfrequentAccessRealStorage");

    if (node) {
        result.infrequentAccessRealStorage = toInt64(node);
    }


    node = root->FirstChildElement("ArchiveObjectCount");

    if (node) {
        result.archiveObjectCount = toInt64(node);
    }


    node = root->FirstChildElement("Storage");

    if (node) {
        result.storage = toInt64(node);
    }


    node = root->FirstChildElement("ObjectCount");

    if (node) {
        result.objectCount = toInt64(node);
    }


    node = root->FirstChildElement("InfrequentAccessObjectCount");

    if (node) {
        result.infrequentAccessObjectCount = toInt64(node);
    }


    node = root->FirstChildElement("ColdArchiveMultipartPartCount");

    if (node) {
        result.coldArchiveMultipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("LiveChannelCount");

    if (node) {
        result.liveChannelCount = toInt64(node);
    }


    node = root->FirstChildElement("LastModifiedTime");

    if (node) {
        result.lastModifiedTime = toInt64(node);
    }


    node = root->FirstChildElement("ArchiveRealStorage");

    if (node) {
        result.archiveRealStorage = toInt64(node);
    }


    node = root->FirstChildElement("DeepColdArchiveStorage");

    if (node) {
        result.deepColdArchiveStorage = toInt64(node);
    }


    node = root->FirstChildElement("DeepColdArchiveRealStorage");

    if (node) {
        result.deepColdArchiveRealStorage = toInt64(node);
    }


    node = root->FirstChildElement("DeepColdArchiveObjectCount");

    if (node) {
        result.deepColdArchiveObjectCount = toInt64(node);
    }


    node = root->FirstChildElement("MultipartPartStorage");

    if (node) {
        result.multipartPartStorage = toInt64(node);
    }


    node = root->FirstChildElement("MultipartUploadCount");

    if (node) {
        result.multipartUploadCount = toInt64(node);
    }


    node = root->FirstChildElement("StandardStorage");

    if (node) {
        result.standardStorage = toInt64(node);
    }


    node = root->FirstChildElement("StandardMultipartPartStorage");

    if (node) {
        result.standardMultipartPartStorage = toInt64(node);
    }


    node = root->FirstChildElement("ArchiveMultipartPartCount");

    if (node) {
        result.archiveMultipartPartCount = toInt64(node);
    }


    node = root->FirstChildElement("ArchiveMultipartPartStorage");

    if (node) {
        result.archiveMultipartPartStorage = toInt64(node);
    }


    node = root->FirstChildElement("DeepColdArchiveMultipartPartStorage");

    if (node) {
        result.deepColdArchiveMultipartPartStorage = toInt64(node);
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


inline static models::BucketPolicy toBucketPolicy(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::BucketPolicy();

    node = root->FirstChildElement("LogBucket");

    if (node) {
        result.logBucket = toString(node);
    }


    node = root->FirstChildElement("LogPrefix");

    if (node) {
        result.logPrefix = toString(node);
    }


    return result;
}


inline static std::string toXmlText(const models::CreateBucketConfiguration& value, const std::string& tag) {
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

inline static models::CommonPrefix toCommonPrefix(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::CommonPrefix();

    node = root->FirstChildElement("Prefix");

    if (node) {
        result.prefix = toString(node);
    }


    return result;
}


inline static models::ServerSideEncryptionRule toServerSideEncryptionRule(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ServerSideEncryptionRule();

    node = root->FirstChildElement("KMSMasterKeyID");

    if (node) {
        result.kmsMasterKeyID = toString(node);
    }


    node = root->FirstChildElement("KMSDataEncryption");

    if (node) {
        result.kmsDataEncryption = toString(node);
    }


    node = root->FirstChildElement("SSEAlgorithm");

    if (node) {
        result.sseAlgorithm = toString(node);
    }


    return result;
}

inline static models::BucketInfo toBucketInfo(const thirdparty::tinyxml2::XMLElement* root) {
    auto result = models::BucketInfo();
    root = root->FirstChildElement("Bucket");
    if (root) {
        const thirdparty::tinyxml2::XMLElement* node;
        node = root->FirstChildElement("StorageClass");
        if (node) {
            result.storageClass = toString(node);
        }

        node = root->FirstChildElement("Comment");
        if (node) {
            result.comment = toString(node);
        }

        node = root->FirstChildElement("DataRedundancyType");
        if (node) {
            result.dataRedundancyType = toString(node);
        }

        node = root->FirstChildElement("Location");
        if (node) {
            result.location = toString(node);
        }

        node = root->FirstChildElement("Name");
        if (node) {
            result.name = toString(node);
        }

        node = root->FirstChildElement("ResourceGroupId");
        if (node) {
            result.resourceGroupId = toString(node);
        }

        node = root->FirstChildElement("TransferAcceleration");
        if (node) {
            result.transferAcceleration = toString(node);
        }

        node = root->FirstChildElement("Owner");
        if (node) {
            result.owner = toOwner(node);
        }

        node = root->FirstChildElement("BlockPublicAccess");
        if (node) {
            result.blockPublicAccess = toBool(node);
        }

        node = root->FirstChildElement("CreationDate");
        if (node) {
            result.creationDate = toString(node);
        }

        node = root->FirstChildElement("CrossRegionReplication");
        if (node) {
            result.crossRegionReplication = toString(node);
        }

        node = root->FirstChildElement("AccessControlList");
        if (node) {
            result.accessControlList = toAccessControlList(node);
        }

        node = root->FirstChildElement("ServerSideEncryptionRule");
        if (node) {
            result.serverSideEncryptionRule = toServerSideEncryptionRule(node);
        }

        node = root->FirstChildElement("AccessMonitor");
        if (node) {
            result.accessMonitor = toString(node);
        }

        node = root->FirstChildElement("IntranetEndpoint");
        if (node) {
            result.intranetEndpoint = toString(node);
        }

        node = root->FirstChildElement("Versioning");
        if (node) {
            result.versioning = toString(node);
        }

        node = root->FirstChildElement("BucketPolicy");
        if (node) {
            result.bucketPolicy = toBucketPolicy(node);
        }

        node = root->FirstChildElement("ExtranetEndpoint");
        if (node) {
            result.extranetEndpoint = toString(node);
        }
    }
    return result;
}


inline static models::ObjectSummary toObjectSummary(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ObjectSummary();

    node = root->FirstChildElement("LastModified");

    if (node) {
        result.lastModified = toString(node);
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


    node = root->FirstChildElement("TransitionTime");

    if (node) {
        result.transitionTime = toString(node);
    }


    node = root->FirstChildElement("Key");

    if (node) {
        result.key = toString(node);
    }


    node = root->FirstChildElement("ETag");

    if (node) {
        result.eTag = toString(node);
    }


    node = root->FirstChildElement("Type");

    if (node) {
        result.type = toString(node);
    }


    return result;
}


inline static models::ListBucketResultXml toListBucketResult(const thirdparty::tinyxml2::XMLElement* root) {
    const thirdparty::tinyxml2::XMLElement* node;
    auto result = models::ListBucketResultXml();

    node = root->FirstChildElement("EncodingType");
    if (node) {
        result.encodingType = toString(node);
    }
    bool doDecode = result.encodingType == "url";

    node = root->FirstChildElement("Marker");
    if (node) {
        result.marker = toString(node, doDecode);
    }

    node = root->FirstChildElement("Delimiter");
    if (node) {
        result.delimiter = toString(node, doDecode);
    }

    node = root->FirstChildElement("NextMarker");
    if (node) {
        result.nextMarker = toString(node, doDecode);
    }

    node = root->FirstChildElement("Name");
    if (node) {
        result.name = toString(node);
    }

    node = root->FirstChildElement("Prefix");
    if (node) {
        result.prefix = toString(node, doDecode);
    }

    node = root->FirstChildElement("MaxKeys");
    if (node) {
        result.maxKeys = toInt32(node);
    }

    node = root->FirstChildElement("IsTruncated");
    if (node) {
        result.isTruncated = toBool(node);
    }

    node = root->FirstChildElement("Contents");
    for (; node; node = node->NextSiblingElement("Contents")) {
        result.contents.emplace_back(toObjectSummary(node));
        if (doDecode) {
            auto& item = result.contents.back();
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

    node = root->FirstChildElement("NextContinuationToken");
    if (node) {
        result.nextContinuationToken = toString(node, doDecode);
    }

    node = root->FirstChildElement("ContinuationToken");
    if (node) {
        result.continuationToken = toString(node, doDecode);
    }

    node = root->FirstChildElement("StartAfter");
    if (node) {
        result.startAfter = toString(node, doDecode);
    }

    node = root->FirstChildElement("KeyCount");
    if (node) {
        result.keyCount = toInt32(node);
    }

    return result;
}

OperationInput fromGetBucketStat(const models::GetBucketStatRequest& request) {
    auto input = OperationInput{"GetBucketStat", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("stat", "");


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

Outcome<models::GetBucketStatResult, OperationError> toGetBucketStat(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetBucketStatResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("BucketStat", root->Name())) {
                result.setBucketStat(toBucketStat(root));
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

    return models::GetBucketStatResult(output.statusCode, std::move(output.headers));
}


OperationInput fromPutBucket(const models::PutBucketRequest& request) {
    auto input = OperationInput{"PutBucket", "PUT"};

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

    if (request.hasCreateBucketConfiguration()) {
        auto str = toXmlText(request.getCreateBucketConfiguration(), "CreateBucketConfiguration");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::FromString(std::move(str));
    }

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();


    return input;
}

Outcome<models::PutBucketResult, OperationError> toPutBucket(OperationOutput&& output) {
    return models::PutBucketResult(output.statusCode, std::move(output.headers));
}


OperationInput fromDeleteBucket(const models::DeleteBucketRequest& request) {
    auto input = OperationInput{"DeleteBucket", "DELETE"};

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


    return input;
}

Outcome<models::DeleteBucketResult, OperationError> toDeleteBucket(OperationOutput&& output) {
    return models::DeleteBucketResult(output.statusCode, std::move(output.headers));
}


OperationInput fromListObjects(const models::ListObjectsRequest& request) {
    auto input = OperationInput{"ListObjects", "GET"};

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


    return input;
}

Outcome<models::ListObjectsResult, OperationError> toListObjects(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListBucketResult", root->Name())) {
                return models::ListObjectsResult(output.statusCode, std::move(output.headers),
                                                 toListBucketResult(root));
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

    return models::ListObjectsResult(output.statusCode, std::move(output.headers));
}


OperationInput fromListObjectsV2(const models::ListObjectsV2Request& request) {
    auto input = OperationInput{"ListObjectsV2", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters
    input.parameters.emplace("list-type", "2");
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

Outcome<models::ListObjectsV2Result, OperationError> toListObjectsV2(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("ListBucketResult", root->Name())) {
                return models::ListObjectsV2Result(output.statusCode, std::move(output.headers),
                                                   toListBucketResult(root));
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

    return models::ListObjectsV2Result(output.statusCode, std::move(output.headers));
}


OperationInput fromGetBucketInfo(const models::GetBucketInfoRequest& request) {
    auto input = OperationInput{"GetBucketInfo", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("bucketInfo", "");


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

Outcome<models::GetBucketInfoResult, OperationError> toGetBucketInfo(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            auto result = models::GetBucketInfoResult(output.statusCode, std::move(output.headers));
            if (root != nullptr && !std::strcmp("BucketInfo", root->Name())) {
                result.setBucketInfo(toBucketInfo(root));
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

    return models::GetBucketInfoResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetBucketLocation(const models::GetBucketLocationRequest& request) {
    auto input = OperationInput{"GetBucketLocation", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("location", "");


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

Outcome<models::GetBucketLocationResult, OperationError> toGetBucketLocation(OperationOutput&& output) {
    if (output.body != nullptr) {
        thirdparty::tinyxml2::XMLDocument doc;
        thirdparty::tinyxml2::XMLError xml_err;
        std::istreambuf_iterator<char> isb(*output.body.get()), end;
        std::string str(isb, end);
        if ((xml_err = doc.Parse(str.c_str(), str.size())) == thirdparty::tinyxml2::XML_SUCCESS) {
            const auto* root = doc.RootElement();
            if (root != nullptr && !std::strcmp("LocationConstraint", root->Name())) {
                auto result = models::GetBucketLocationResult(output.statusCode, std::move(output.headers));
                result.setLocationConstraint(toString(root));
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

    return models::GetBucketLocationResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud