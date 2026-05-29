#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>
#include <vector>


namespace alibabacloud {
namespace oss2 {
namespace models {


/*
 * The server-side encryption configurations of the bucket.
 */
struct ALIBABACLOUD_OSS_API ServerSideEncryptionRule final {
    // The default server-side encryption method.Valid values: KMS, AES-256, and SM4.
    std::optional<std::string> sseAlgorithm;

    // The key that is managed by Key Management Service (KMS).
    std::optional<std::string> kmsMasterKeyID;

    // The algorithm that is used to encrypt objects. If you do not configure this parameter, objects are encrypted by
    // using AES-256. This parameter is valid only when SSEAlgorithm is set to KMS.Valid value: SM4.
    std::optional<std::string> kmsDataEncryption;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    ServerSideEncryptionRule& setSSEAlgorithm(ValueT&& value) {
        sseAlgorithm = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ServerSideEncryptionRule& setKMSMasterKeyID(ValueT&& value) {
        kmsMasterKeyID = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ServerSideEncryptionRule& setKMSDataEncryption(ValueT&& value) {
        kmsDataEncryption = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores all information returned for the GetBucketStat request.
 */
struct ALIBABACLOUD_OSS_API BucketStat final {
    // The total number of objects in the bucket.
    std::optional<std::int64_t> objectCount;

    // The billed storage usage of Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> archiveStorage;

    // The number of Cold Archive objects in the bucket.
    std::optional<std::int64_t> coldArchiveObjectCount;

    // The number of deletemarker in the bucket.
    std::optional<std::int64_t> deleteMarkerCount;

    // The storage usage of Standard objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> standardStorage;

    // The actual storage usage of IA objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> infrequentAccessRealStorage;

    // The storage usage of Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> multipartPartStorage;

    // The storage usage of IA Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> infrequentMultipartPartStorage;

    // The number of Archive Multipart parts in the bucket.
    std::optional<std::int64_t> archiveMultipartPartCount;

    // The number of Deep Cold Archive Multipart parts in the bucket.
    std::optional<std::int64_t> deepColdArchiveMultipartPartCount;

    // The actual storage usage of Deep Cold Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> deepColdArchiveRealStorage;

    // The number of IA Multipart parts in the bucket.
    std::optional<std::int64_t> infrequentMultipartPartCount;

    // The billed storage usage of IA objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> infrequentAccessStorage;

    // The number of Archive objects in the bucket.
    std::optional<std::int64_t> archiveObjectCount;

    // The actual storage usage of Cold Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> coldArchiveRealStorage;

    // The billed storage usage of Deep Cold Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> deepColdArchiveStorage;

    // The number of Deep Cold Archive objects in the bucket.
    std::optional<std::int64_t> deepColdArchiveObjectCount;

    // The storage usage of the bucket. Unit: bytes.
    std::optional<std::int64_t> storage;

    // The number of LiveChannels in the bucket.
    std::optional<std::int64_t> liveChannelCount;

    // The time when the obtained information was last modified. The value of this parameter is a UNIX timestamp. Unit:
    // seconds.
    std::optional<std::int64_t> lastModifiedTime;

    // The storage usage of Deep Cold Archive Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> deepColdArchiveMultipartPartStorage;

    // The number of Standard objects in the bucket.
    std::optional<std::int64_t> standardObjectCount;

    // The actual storage usage of Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> archiveRealStorage;

    // The storage usage of Archive Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> archiveMultipartPartStorage;

    // The number of multipart upload tasks that have been initiated but are not completed or canceled.
    std::optional<std::int64_t> multipartUploadCount;

    // The number of mulitpart parts in the bucket.
    std::optional<std::int64_t> multipartPartCount;

    // The number of IA objects in the bucket.
    std::optional<std::int64_t> infrequentAccessObjectCount;

    // The number of Cold Archive Multipart parts in the bucket.
    std::optional<std::int64_t> coldArchiveMultipartPartCount;

    // The storage usage of Cold Archive Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> coldArchiveMultipartPartStorage;

    // The billed storage usage of Cold Archive objects in the bucket. Unit: bytes.
    std::optional<std::int64_t> coldArchiveStorage;

    // The number of Standard Multipart parts in the bucket.
    std::optional<std::int64_t> standardMultipartPartCount;

    // The storage usage of Standard Multipart parts in the bucket. Unit: bytes.
    std::optional<std::int64_t> standardMultipartPartStorage;


    // Provide setter interfaces via template
    template <typename ValueT = std::int64_t>
    BucketStat& setObjectCount(ValueT&& value) {
        objectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setArchiveStorage(ValueT&& value) {
        archiveStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setColdArchiveObjectCount(ValueT&& value) {
        coldArchiveObjectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeleteMarkerCount(ValueT&& value) {
        deleteMarkerCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setStandardStorage(ValueT&& value) {
        standardStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setInfrequentAccessRealStorage(ValueT&& value) {
        infrequentAccessRealStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setMultipartPartStorage(ValueT&& value) {
        multipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setInfrequentMultipartPartStorage(ValueT&& value) {
        infrequentMultipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setArchiveMultipartPartCount(ValueT&& value) {
        archiveMultipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeepColdArchiveMultipartPartCount(ValueT&& value) {
        deepColdArchiveMultipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeepColdArchiveRealStorage(ValueT&& value) {
        deepColdArchiveRealStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setInfrequentMultipartPartCount(ValueT&& value) {
        infrequentMultipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setInfrequentAccessStorage(ValueT&& value) {
        infrequentAccessStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setArchiveObjectCount(ValueT&& value) {
        archiveObjectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setColdArchiveRealStorage(ValueT&& value) {
        coldArchiveRealStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeepColdArchiveStorage(ValueT&& value) {
        deepColdArchiveStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeepColdArchiveObjectCount(ValueT&& value) {
        deepColdArchiveObjectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setStorage(ValueT&& value) {
        storage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setLiveChannelCount(ValueT&& value) {
        liveChannelCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setLastModifiedTime(ValueT&& value) {
        lastModifiedTime = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setDeepColdArchiveMultipartPartStorage(ValueT&& value) {
        deepColdArchiveMultipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setStandardObjectCount(ValueT&& value) {
        standardObjectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setArchiveRealStorage(ValueT&& value) {
        archiveRealStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setArchiveMultipartPartStorage(ValueT&& value) {
        archiveMultipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setMultipartUploadCount(ValueT&& value) {
        multipartUploadCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setMultipartPartCount(ValueT&& value) {
        multipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setInfrequentAccessObjectCount(ValueT&& value) {
        infrequentAccessObjectCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setColdArchiveMultipartPartCount(ValueT&& value) {
        coldArchiveMultipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setColdArchiveMultipartPartStorage(ValueT&& value) {
        coldArchiveMultipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setColdArchiveStorage(ValueT&& value) {
        coldArchiveStorage = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setStandardMultipartPartCount(ValueT&& value) {
        standardMultipartPartCount = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    BucketStat& setStandardMultipartPartStorage(ValueT&& value) {
        standardMultipartPartStorage = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The log configurations of the bucket.
 */
struct ALIBABACLOUD_OSS_API BucketPolicy final {
    // The directory used to store access logs.
    std::optional<std::string> logPrefix;

    // The name of the bucket used to store access logs.
    std::optional<std::string> logBucket;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    BucketPolicy& setLogPrefix(ValueT&& value) {
        logPrefix = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketPolicy& setLogBucket(ValueT&& value) {
        logBucket = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The configurations of the bucket storage class and redundancy type.
 */
struct ALIBABACLOUD_OSS_API CreateBucketConfiguration final {
    // The storage class of the bucket. Valid values:*   Standard (default)*   IA*   Archive*   ColdArchive
    std::optional<std::string> storageClass;

    // The redundancy type of the bucket.*   LRS (default)    LRS stores multiple copies of your data on multiple
    // devices in the same zone. LRS ensures data durability and availability even if hardware failures occur on two
    // devices.*   ZRS    ZRS stores multiple copies of your data across three zones in the same region. Even if a zone
    // becomes unavailable due to unexpected events, such as power outages and fires, data can still be accessed.  You
    // cannot set the redundancy type of Archive buckets to ZRS.
    std::optional<std::string> dataRedundancyType;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    CreateBucketConfiguration& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    CreateBucketConfiguration& setDataRedundancyType(ValueT&& value) {
        dataRedundancyType = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The object metadata returned.
 */
struct ALIBABACLOUD_OSS_API ObjectSummary final {
    // The key of the object.
    std::string key;

    // The types of the returned objects.*   Normal: Objects created by using simple upload.*   Multipart: Objects
    // created by using multipart upload.*   Appendable: Objects created by using append upload. You can append content
    // to objects only of the Appendable type.
    std::string type;

    // The sizes of the returned objects. Unit: byte.
    std::int64_t size;

    // The time when the object was last modified.
    std::string lastModified;

    // The ETag that is generated when an object is created. ETags are used to identify the content of objects.*   For
    // an object that is created by calling the PutObject operation, the ETag value of the object is the MD5 hash of its
    // content.*   For an object that is created by using another method, the ETag value is not the MD5 hash of the
    // object content but a unique value calculated based on a specific rule.*   The ETag of an object can be used to
    // check whether the object content is modified. We recommend that you use the MD5 hash of an object rather than the
    // ETag of it to verify data integrity.
    std::string eTag;

    // The storage class of the object.
    std::string storageClass;

    // The container for the information about the bucket owner.
    std::optional<Owner> owner;

    // The restoration status of the object.
    std::optional<std::string> restoreInfo;

    // The time when the storage class of the object is converted to Cold Archive or Deep Cold Archive based on
    // lifecycle rules.
    std::optional<std::string> transitionTime;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    ObjectSummary& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setType(ValueT&& value) {
        type = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    ObjectSummary& setSize(ValueT&& value) {
        size = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setLastModified(ValueT&& value) {
        lastModified = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setETag(ValueT&& value) {
        eTag = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = Owner>
    ObjectSummary& setOwner(ValueT&& value) {
        owner = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setRestoreInfo(ValueT&& value) {
        restoreInfo = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectSummary& setTransitionTime(ValueT&& value) {
        transitionTime = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The container that stores the bucket information.
 */
struct ALIBABACLOUD_OSS_API BucketInfo final {
    // The region in which the bucket is located.
    std::string location;

    // The name of the bucket.
    std::string name;

    // The storage class of the bucket.
    std::string storageClass;

    // The redundancy type of the bucket.
    std::string dataRedundancyType;

    // The time when the bucket is created.
    std::string creationDate;

    // The public endpoint of the bucket.
    std::string extranetEndpoint;

    // The internal endpoint of the bucket.
    std::string intranetEndpoint;

    // Bucket description.
    std::string comment;

    // The owner of the bucket.
    Owner owner;

    // Indicates whether transfer acceleration is enabled for the bucket.Valid values:*   Enabled            * Disabled
    std::optional<std::string> transferAcceleration;

    // Indicates whether access tracking is enabled for the bucket.Valid values:*   Enabled            *   Disabled
    std::optional<std::string> accessMonitor;

    // The ID of the resource group to which the bucket belongs.
    std::optional<std::string> resourceGroupId;

    // The ACL of the bucket.
    std::optional<AccessControlList> accessControlList;

    // Whether the bucket has been configured to block public access.
    std::optional<bool> blockPublicAccess;

    // Indicates whether cross-region replication (CRR) is enabled for the bucket.Valid values:*   Enabled            *
    // Disabled
    std::optional<std::string> crossRegionReplication;

    // The server-side encryption configurations of the bucket.
    std::optional<ServerSideEncryptionRule> serverSideEncryptionRule;

    // The log configurations of the bucket.
    std::optional<BucketPolicy> bucketPolicy;

    // The versioning status of the bucket.
    std::optional<std::string> versioning;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    BucketInfo& setLocation(ValueT&& value) {
        location = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setName(ValueT&& value) {
        name = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setTransferAcceleration(ValueT&& value) {
        transferAcceleration = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setAccessMonitor(ValueT&& value) {
        accessMonitor = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setDataRedundancyType(ValueT&& value) {
        dataRedundancyType = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setResourceGroupId(ValueT&& value) {
        resourceGroupId = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = AccessControlList>
    BucketInfo& setAccessControlList(ValueT&& value) {
        accessControlList = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = bool>
    BucketInfo& setBlockPublicAccess(ValueT&& value) {
        blockPublicAccess = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setCreationDate(ValueT&& value) {
        creationDate = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setCrossRegionReplication(ValueT&& value) {
        crossRegionReplication = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setExtranetEndpoint(ValueT&& value) {
        extranetEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setIntranetEndpoint(ValueT&& value) {
        intranetEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = ServerSideEncryptionRule>
    BucketInfo& setServerSideEncryptionRule(ValueT&& value) {
        serverSideEncryptionRule = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = BucketPolicy>
    BucketInfo& setBucketPolicy(ValueT&& value) {
        bucketPolicy = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setComment(ValueT&& value) {
        comment = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketInfo& setVersioning(ValueT&& value) {
        versioning = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = Owner>
    BucketInfo& setOwner(ValueT&& value) {
        owner = std::forward<ValueT>(value);
        return *this;
    }
};


// The request for the GetBucketStat operation.
class ALIBABACLOUD_OSS_API GetBucketStatRequest final : public RequestModel {
  public:
    GetBucketStatRequest() = default;

    // The bucket about which you want to query the information.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketStatRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketStat operation.
class ALIBABACLOUD_OSS_API GetBucketStatResult final : public ResultModel {
  public:
    GetBucketStatResult() = default;
    GetBucketStatResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores all information returned for the GetBucketStat request.
    inline const BucketStat& getBucketStat() {
        return body_[0];
    }

    inline bool hasBucketStat() const {
        return bodyIsSet_;
    }

    template <typename ValueT = BucketStat>
    GetBucketStatResult& setBucketStat(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, BucketStat> body_;
    bool bodyIsSet_{};
};

// The request for the PutBucket operation.
class ALIBABACLOUD_OSS_API PutBucketRequest final : public RequestModel {
  public:
    PutBucketRequest() = default;

    // The name of the bucket. The name of a bucket must comply with the following naming conventions:*   The name can
    // contain only lowercase letters, digits, and hyphens (-).*   It must start and end with a lowercase letter or a
    // digit.*   The name must be 3 to 63 characters in length.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutBucketRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The access control list (ACL) of the bucket to be created. Valid values:*   public-read-write*   public-read*
    // private (default)For more information, see [Bucket ACL](~~31843~~).
    inline const std::string& getAcl() const {
        return getHeaderOrEmpty("x-oss-acl");
    }
    template <typename ValueT = std::string>
    PutBucketRequest& setAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The ID of the resource group.*   If you include the header in the request and specify the ID of the resource
    // group, the bucket that you create belongs to the resource group. If the specified resource group ID is
    // rg-default-id, the bucket that you create belongs to the default resource group.*   If you do not include the
    // header in the request, the bucket that you create belongs to the default resource group.You can obtain the ID of
    // a resource group in the Resource Management console or by calling the ListResourceGroups operation. For more
    // information, see [View basic information of a resource group](~~151181~~) and [ListResourceGroups](~~158855~~).
    // You cannot configure a resource group for an Anywhere Bucket.
    inline const std::string& getResourceGroupId() const {
        return getHeaderOrEmpty("x-oss-resource-group-id");
    }
    template <typename ValueT = std::string>
    PutBucketRequest& setResourceGroupId(ValueT&& value) {
        headers_.insert_or_assign("x-oss-resource-group-id", std::forward<ValueT>(value));
        return *this;
    }

    // The Bucket tagging，for example k1=v1&k2=v2
    inline const std::string& getBucketTagging() const {
        return getHeaderOrEmpty("x-oss-bucket-tagging");
    }
    template <typename ValueT = std::string>
    PutBucketRequest& setBucketTagging(ValueT&& value) {
        headers_.insert_or_assign("x-oss-bucket-tagging", std::forward<ValueT>(value));
        return *this;
    }

    // The container that stores the request body.
    inline const CreateBucketConfiguration& getCreateBucketConfiguration() const {
        return body_.at(0);
    }

    inline bool hasCreateBucketConfiguration() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = CreateBucketConfiguration>
    PutBucketRequest& setCreateBucketConfiguration(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::map<int, CreateBucketConfiguration> body_;
};

/// The result for the PutBucket operation.
class ALIBABACLOUD_OSS_API PutBucketResult final : public ResultModel {
  public:
    PutBucketResult() = default;
    PutBucketResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the DeleteBucket operation.
class ALIBABACLOUD_OSS_API DeleteBucketRequest final : public RequestModel {
  public:
    DeleteBucketRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    DeleteBucketRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the DeleteBucket operation.
class ALIBABACLOUD_OSS_API DeleteBucketResult final : public ResultModel {
  public:
    DeleteBucketResult() = default;
    DeleteBucketResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the ListObjects operation.
class ALIBABACLOUD_OSS_API ListObjectsRequest final : public RequestModel {
  public:
    ListObjectsRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    ListObjectsRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The character that is used to group objects by name. If you specify delimiter in the request, the response
    // contains CommonPrefixes. The objects whose names contain the same string from the prefix to the next occurrence
    // of the delimiter are grouped as a single result element in CommonPrefixes.
    inline const std::string& getDelimiter() const {
        return getParameterOrEmpty("delimiter");
    }
    template <typename ValueT = std::string>
    ListObjectsRequest& setDelimiter(ValueT&& value) {
        parameters_.insert_or_assign("delimiter", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the object after which the getBucket (ListObjects) operation begins. If this parameter is specified,
    // objects whose names are alphabetically after the value of marker are returned.The objects are returned by page
    // based on marker. The value of marker can be up to 1,024 bytes.If the value of marker does not exist in the list
    // when you perform a conditional query, the getBucket (ListObjects) operation starts from the object whose name is
    // alphabetically after the value of marker.
    inline const std::string& getMarker() const {
        return getParameterOrEmpty("marker");
    }
    template <typename ValueT = std::string>
    ListObjectsRequest& setMarker(ValueT&& value) {
        parameters_.insert_or_assign("marker", std::forward<ValueT>(value));
        return *this;
    }

    // The maximum number of objects that can be returned. If the number of objects to be returned exceeds the value of
    // max-keys specified in the request, NextMarker is included in the returned response. The value of NextMarker is
    // used as the value of marker for the next request.Valid values: 1 to 999.Default value: 100.
    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    template <typename ValueT = std::int64_t>
    ListObjectsRequest& setMaxKeys(ValueT&& value) {
        parameters_.insert_or_assign("max-keys", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The prefix that must be contained in names of the returned objects.*   The value of prefix can be up to 1,024
    // bytes in length.*   If you specify prefix, the names of the returned objects contain the prefix.If you set prefix
    // to a directory name, the object whose names start with this prefix are listed. The objects consist of all
    // recursive objects and subdirectories in this directory.If you set prefix to a directory name and set delimiter to
    // a forward slash (/), only the objects in the directory are listed. The subdirectories in the directory are listed
    // in CommonPrefixes. Recursive objects and subdirectories in the subdirectories are not listed.For example, a
    // bucket contains the following three objects: fun/test.jpg, fun/movie/001.avi, and fun/movie/007.avi. If prefix is
    // set to fun/, the three objects are returned. If prefix is set to fun/ and delimiter is set to a forward slash
    // (/), fun/test.jpg and fun/movie/ are returned.
    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListObjectsRequest& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding format of the content in the response.  The value of Delimiter, Marker, Prefix, NextMarker, and Key
    // are UTF-8 encoded. If the values of Delimiter, Marker, Prefix, NextMarker, and Key contain a control character
    // that is not supported by Extensible Markup Language (XML) 1.0, you can specify encoding-type to encode the value
    // in the response.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    ListObjectsRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
};


/*
 * The container that stores the result of the getBucket (ListObjects) request.
 */
struct ALIBABACLOUD_OSS_API ListBucketResultXml final {
    std::string name;
    std::string encodingType;
    std::string nextMarker;
    std::string prefix;
    std::string marker;
    std::string delimiter;
    std::optional<std::int32_t> maxKeys;
    std::optional<bool> isTruncated;
    std::vector<ObjectSummary> contents;
    std::vector<CommonPrefix> commonPrefixes;

    // list objects v2
    std::string startAfter;
    std::string continuationToken;
    std::string nextContinuationToken;
    std::optional<std::int32_t> keyCount;
};


/// The result for the ListObjects operation.
class ALIBABACLOUD_OSS_API ListObjectsResult final : public ResultModel {
  public:
    ListObjectsResult() = default;
    ListObjectsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    ListObjectsResult(int statusCode, HeaderCollection headers, ListBucketResultXml body)
        : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}


    // The name of the bucket.
    inline const std::string& getName() const {
        return body_.name;
    }

    // The encoding type of the content in the response. If the encoding-type parameter is specified in the request, the
    // values of Delimiter, Marker, Prefix, NextMarker, and Key in the response are encoded.
    inline const std::string& getEncodingType() const {
        return body_.encodingType;
    }

    // The position from which the next list operation starts.
    inline const std::string& getNextMarker() const {
        return body_.nextMarker;
    }

    // The container that stores the metadata of each returned object.
    inline const std::vector<ObjectSummary>& getContents() const {
        return body_.contents;
    }

    // If the delimiter parameter is specified in the request, the response contains CommonPrefixes. Objects whose names
    // contain the same string from the prefix to the next occurrence of the delimiter are grouped as a single result
    // element in CommonPrefixes.
    inline const std::vector<CommonPrefix>& getCommonPrefixes() const {
        return body_.commonPrefixes;
    }

    // The prefix in the names of the returned objects.
    inline const std::string& getPrefix() const {
        return body_.prefix;
    }

    // The name of the object after which the list operation starts.
    inline const std::string& getMarker() const {
        return body_.marker;
    }

    // The maximum number of the returned objects in the response.
    inline std::int32_t getMaxKeys() const {
        return body_.maxKeys.value_or(-1);
    }

    // The delimiter used to group objects by name. Objects whose names contain the same string from the prefix to the
    // next occurrence of the delimiter are grouped as a single result element in the CommonPrefixes parameter.
    inline const std::string& getDelimiter() const {
        return body_.delimiter;
    }

    // Indicates whether the returned results are truncated.Valid values: true and falsetrue: indicates that not all of
    // the results are returned for the request.false indicates that all of the results are returned this time.**
    inline bool getIsTruncated() const {
        return body_.isTruncated.value_or(false);
    }


  private:
    ListBucketResultXml body_;
};


// The request for the ListObjectsV2 operation.
class ALIBABACLOUD_OSS_API ListObjectsV2Request final : public RequestModel {
  public:
    ListObjectsV2Request() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    ListObjectsV2Request& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The character that is used to group objects by name. If you specify delimiter in the request, the response
    // contains CommonPrefixes. The objects whose names contain the same string from the prefix to the next occurrence
    // of the delimiter are grouped as a single result element in CommonPrefixes.
    inline const std::string& getDelimiter() const {
        return getParameterOrEmpty("delimiter");
    }
    template <typename ValueT = std::string>
    ListObjectsV2Request& setDelimiter(ValueT&& value) {
        parameters_.insert_or_assign("delimiter", std::forward<ValueT>(value));
        return *this;
    }

    // The maximum number of objects to be returned.Valid values: 1 to 999.Default value: 100.  If the number of
    // returned objects exceeds the value of max-keys, the response contains NextContinuationToken.Use the value of
    // NextContinuationToken as the value of continuation-token in the next request.
    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    template <typename ValueT = std::int64_t>
    ListObjectsV2Request& setMaxKeys(ValueT&& value) {
        parameters_.insert_or_assign("max-keys", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The prefix that must be contained in names of the returned objects.*   The value of prefix can be up to 1,024
    // bytes in length.*   If you specify prefix, the names of the returned objects contain the prefix.If you set prefix
    // to a directory name, the objects whose names start with this prefix are listed. The objects consist of all
    // objects and subdirectories in this directory.If you set prefix to a directory name and set delimiter to a forward
    // slash (/), only the objects in the directory are listed. The subdirectories in the directory are returned in
    // CommonPrefixes. Objects and subdirectories in the subdirectories are not listed.For example, a bucket contains
    // the following three objects: fun/test.jpg, fun/movie/001.avi, and fun/movie/007.avi. If prefix is set to fun/,
    // the three objects are returned. If prefix is set to fun/ and delimiter is set to a forward slash (/),
    // fun/test.jpg and fun/movie/ are returned.
    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListObjectsV2Request& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding format of the returned objects in the response.  The values of Delimiter, StartAfter, Prefix,
    // NextContinuationToken, and Key are UTF-8 encoded. If the value of Delimiter, StartAfter, Prefix,
    // NextContinuationToken, or Key contains a control character that is not supported by Extensible Markup Language
    // (XML) 1.0, you can specify encoding-type to encode the value in the response.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    ListObjectsV2Request& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }

    // Specifies whether to include the information about the bucket owner in the response. Valid values:*   true* false
    inline bool getFetchOwner() const {
        return getParameterAsBoolOr("fetch-owner");
    }
    template <typename ValueT = bool>
    ListObjectsV2Request& setFetchOwner(ValueT&& value) {
        parameters_.insert_or_assign("fetch-owner", std::forward<ValueT>(value) ? "true" : "false");
        return *this;
    }

    // The name of the object after which the list operation begins. If this parameter is specified, objects whose names
    // are alphabetically after the value of start-after are returned.The objects are returned by page based on
    // start-after. The value of start-after can be up to 1,024 bytes in length.If the value of start-after does not
    // exist when you perform a conditional query, the list starts from the object whose name is alphabetically after
    // the value of start-after.
    inline const std::string& getStartAfter() const {
        return getParameterOrEmpty("start-after");
    }
    template <typename ValueT = std::string>
    ListObjectsV2Request& setStartAfter(ValueT&& value) {
        parameters_.insert_or_assign("start-after", std::forward<ValueT>(value));
        return *this;
    }

    // The token from which the list operation starts. You can obtain the token from NextContinuationToken in the
    // response of the ListObjectsV2 request.
    inline const std::string& getContinuationToken() const {
        return getParameterOrEmpty("continuation-token");
    }
    template <typename ValueT = std::string>
    ListObjectsV2Request& setContinuationToken(ValueT&& value) {
        parameters_.insert_or_assign("continuation-token", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the ListObjectsV2 operation.
class ALIBABACLOUD_OSS_API ListObjectsV2Result final : public ResultModel {
  public:
    ListObjectsV2Result() = default;
    ListObjectsV2Result(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    ListObjectsV2Result(int statusCode, HeaderCollection headers, ListBucketResultXml body)
        : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}


    // The name of the bucket.
    inline const std::string& getName() const {
        return body_.name;
    }

    // The encoding type of the content in the response. If the encoding-type parameter is specified in the request, the
    // values of Delimiter, Marker, Prefix, NextMarker, and Key in the response are encoded.
    inline const std::string& getEncodingType() const {
        return body_.encodingType;
    }

    // The continuation token used in the request.
    inline const std::string& getContinuationToken() const {
        return body_.continuationToken;
    }

    // The position from which the next list operation starts.
    inline const std::string& getNextContinuationToken() const {
        return body_.nextContinuationToken;
    }

    // The container that stores the metadata of each returned object.
    inline const std::vector<ObjectSummary>& getContents() const {
        return body_.contents;
    }

    // If the delimiter parameter is specified in the request, the response contains CommonPrefixes. Objects whose names
    // contain the same string from the prefix to the next occurrence of the delimiter are grouped as a single result
    // element in CommonPrefixes.
    inline const std::vector<CommonPrefix>& getCommonPrefixes() const {
        return body_.commonPrefixes;
    }

    // The prefix in the names of the returned objects.
    inline const std::string& getPrefix() const {
        return body_.prefix;
    }

    // The key to start from for the next list operation.
    inline const std::string& getStartAfter() const {
        return body_.startAfter;
    }

    // The maximum number of the returned objects in the response.
    inline std::int32_t getMaxKeys() const {
        return body_.maxKeys.value_or(-1);
    }

    // The delimiter used to group objects by name. Objects whose names contain the same string from the prefix to the
    // next occurrence of the delimiter are grouped as a single result element in the CommonPrefixes parameter.
    inline const std::string& getDelimiter() const {
        return body_.delimiter;
    }

    // Indicates whether the returned results are truncated.Valid values: true and falsetrue: indicates that not all of
    // the results are returned for the request.false indicates that all of the results are returned this time.**
    inline bool getIsTruncated() const {
        return body_.isTruncated.value_or(false);
    }

    // Gets the total number of keys returned in the response.
    inline std::int32_t getKeyCount() const {
        return body_.keyCount.value_or(-1);
    }

  private:
    ListBucketResultXml body_;
};

// The request for the GetBucketInfo operation.
class ALIBABACLOUD_OSS_API GetBucketInfoRequest final : public RequestModel {
  public:
    GetBucketInfoRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketInfoRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketInfo operation.
class ALIBABACLOUD_OSS_API GetBucketInfoResult final : public ResultModel {
  public:
    GetBucketInfoResult() = default;
    GetBucketInfoResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the information about the bucket.
    inline const BucketInfo& getBucketInfo() {
        return body_[0];
    }

    inline bool hasBucketInfo() const {
        return bodyIsSet_;
    }

    template <typename ValueT = BucketInfo>
    GetBucketInfoResult& setBucketInfo(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, BucketInfo> body_;
    bool bodyIsSet_{};
};

// The request for the GetBucketLocation operation.
class ALIBABACLOUD_OSS_API GetBucketLocationRequest final : public RequestModel {
  public:
    GetBucketLocationRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketLocationRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketLocation operation.
class ALIBABACLOUD_OSS_API GetBucketLocationResult final : public ResultModel {
  public:
    GetBucketLocationResult() = default;
    GetBucketLocationResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The region in which the bucket resides.Examples: oss-cn-hangzhou, oss-cn-shanghai, oss-cn-qingdao,
    // oss-cn-beijing, oss-cn-zhangjiakou, oss-cn-hongkong, oss-cn-shenzhen, oss-us-west-1, oss-us-east-1, and
    // oss-ap-southeast-1.For more information about the regions in which buckets reside.
    inline const std::string& getLocationConstraint() {
        return locationConstraint_;
    }

    template <typename ValueT = std::string>
    GetBucketLocationResult& setLocationConstraint(ValueT&& value) {
        locationConstraint_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string locationConstraint_;
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
