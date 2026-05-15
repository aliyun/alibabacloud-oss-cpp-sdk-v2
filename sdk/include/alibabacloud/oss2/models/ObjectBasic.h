#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace models {

/*
 * The container that stores the restoration priority coniguration. This configuration takes effect only when the
 * request is sent to restore Cold Archive objects. If you do not specify the JobParameters parameter, the default
 * restoration priority Standard is used.
 */
struct ALIBABACLOUD_OSS_API JobParameters final {
    // The restoration priority. Valid values:*   Expedited: The object is restored within 1 hour.*   Standard: The
    // object is restored within 2 to 5 hours.*   Bulk: The object is restored within 5 to 12 hours.
    std::optional<std::string> tier;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    JobParameters& setTier(ValueT&& value) {
        tier = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The container that stores information about the RestoreObject request.
 */
struct ALIBABACLOUD_OSS_API RestoreRequest final {
    // The duration in which the object can remain in the restored state. Unit: days.
    std::optional<std::int64_t> days;

    // The container that stores the restoration priority coniguration. This configuration takes effect only when the
    // request is sent to restore Cold Archive objects. If you do not specify the JobParameters parameter, the default
    // restoration priority Standard is used.
    std::optional<JobParameters> jobParameters;


    // Provide setter interfaces via template
    template <typename ValueT = std::int64_t>
    RestoreRequest& setDays(ValueT&& value) {
        days = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = JobParameters>
    RestoreRequest& setJobParameters(ValueT&& value) {
        jobParameters = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The identifier of an object to delete.
 */
struct ALIBABACLOUD_OSS_API ObjectIdentifier final {
    // The name of the object.
    std::string key;

    // The version ID of the object.
    std::optional<std::string> versionId;

    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    ObjectIdentifier& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    ObjectIdentifier& setVersionId(ValueT&& value) {
        versionId = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The container for the objects to delete.
 */
struct ALIBABACLOUD_OSS_API Delete final {
    // The name of the object.
    std::vector<ObjectIdentifier> objects;

    // Specifies whether to enable the Quiet return mode.
    std::optional<bool> quiet;

    // Provide setter interfaces via template
    template <typename ValueT = std::vector<ObjectIdentifier>>
    Delete& setObjects(ValueT&& value) {
        objects = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = bool>
    Delete& setQuiet(ValueT&& value) {
        quiet = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The information about a delete object.
 */
struct ALIBABACLOUD_OSS_API DeletedInfo final {
    // The name of the deleted object.
    std::string key;

    // The version ID of the object that you deleted.
    std::optional<std::string> versionId;

    // Indicates whether the deleted version is a delete marker.
    std::optional<bool> deleteMarker;

    // The version ID of the delete marker.
    std::optional<std::string> deleteMarkerVersionId;

    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    DeletedInfo& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    DeletedInfo& setVersionId(ValueT&& value) {
        versionId = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = bool>
    DeletedInfo& setDeleteMarker(ValueT&& value) {
        deleteMarker = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    DeletedInfo& setDeleteMarkerVersionId(ValueT&& value) {
        deleteMarkerVersionId = std::forward<ValueT>(value);
        return *this;
    }

};


// The request for the PutObject operation.
class ALIBABACLOUD_OSS_API PutObjectRequest final : public RequestModel {
  public:
    PutObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // Specifies whether the object that is uploaded by calling the PutObject operation overwrites the existing object
    // that has the same name.  When versioning is enabled or suspended for the bucket to which you want to upload the
    // object, the **x-oss-forbid-overwrite** header does not take effect. In this case, the object that is uploaded by
    // calling the PutObject operation overwrites the existing object that has the same name.   - If you do not specify
    // the **x-oss-forbid-overwrite** header or set the **x-oss-forbid-overwrite** header to **false**, the object that
    // is uploaded by calling the PutObject operation overwrites the existing object that has the same name.   - If the
    // value of **x-oss-forbid-overwrite** is set to **true**, existing objects cannot be overwritten by objects that
    // have the same names. If you specify the **x-oss-forbid-overwrite** request header, the queries per second (QPS)
    // performance of OSS is degraded. If you want to use the **x-oss-forbid-overwrite** request header to perform a
    // large number of operations (QPS greater than 1,000), contact technical support. Default value: **false**.
    inline const std::string& getForbidOverwrite() const {
        return getHeaderOrEmpty("x-oss-forbid-overwrite");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setForbidOverwrite(ValueT&& value) {
        headers_.insert_or_assign("x-oss-forbid-overwrite", std::forward<ValueT>(value));
        return *this;
    }

    // The method that is used to encrypt the object on the OSS server when the object is created. Valid values:
    // **AES256**, **KMS**, and **SM4****.If you specify the header, the header is returned in the response. OSS uses
    // the method that is specified by this header to encrypt the uploaded object. When you download the encrypted
    // object, the **x-oss-server-side-encryption** header is included in the response and the header value is set to
    // the algorithm that is used to encrypt the object.
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setServerSideEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The encryption method on the server side when an object is created. Valid values: **AES256**, **KMS**, and
    // **SM4**.If you specify the header, the header is returned in the response. OSS uses the method that is specified
    // by this header to encrypt the uploaded object. When you download the encrypted object, the
    // **x-oss-server-side-encryption** header is included in the response and the header value is set to the algorithm
    // that is used to encrypt the object.
    inline const std::string& getServerSideDataEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-data-encryption");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setServerSideDataEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-data-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The ID of the customer master key (CMK) managed by Key Management Service (KMS). This header is valid only when
    // the **x-oss-server-side-encryption** header is set to KMS.
    inline const std::string& getServerSideEncryptionKeyId() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption-key-id");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setServerSideEncryptionKeyId(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption-key-id", std::forward<ValueT>(value));
        return *this;
    }

    // The access control list (ACL) of the object. Default value: default. Valid values:- default: The ACL of the
    // object is the same as that of the bucket in which the object is stored. - private: The ACL of the object is
    // private. Only the owner of the object and authorized users can read and write this object. - public-read: The ACL
    // of the object is public-read. Only the owner of the object and authorized users can read and write this object.
    // Other users can only read the object. Exercise caution when you set the object ACL to this value. -
    // public-read-write: The ACL of the object is public-read-write. All users can read and write this object. Exercise
    // caution when you set the object ACL to this value. For more information about the ACL, see **[ACL](~~100676~~)**.
    inline const std::string& getObjectAcl() const {
        return getHeaderOrEmpty("x-oss-object-acl");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setObjectAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-object-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The storage class of the bucket. Default value: Standard.  Valid values:- Standard- IA- Archive- ColdArchive
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setStorageClass(ValueT&& value) {
        headers_.insert_or_assign("x-oss-storage-class", std::forward<ValueT>(value));
        return *this;
    }

    // The tag of the object. You can configure multiple tags for the object. Example: TagA=A&TagB=B.  The key and value
    // of a tag must be URL-encoded. If a tag does not contain an equal sign (=), the value of the tag is considered an
    // empty string.
    inline const std::string& getTagging() const {
        return getHeaderOrEmpty("x-oss-tagging");
    }
    template <typename ValueT = std::string>
    PutObjectRequest& setTagging(ValueT&& value) {
        headers_.insert_or_assign("x-oss-tagging", std::forward<ValueT>(value));
        return *this;
    }

    // <no value>
    inline const HeaderCollection& getMetadata() const {
        return metadata_;
    }
    template <typename ValueT = HeaderCollection>
    PutObjectRequest& setMetadata(ValueT&& value) {
        metadata_ = std::forward<ValueT>(value);
        return *this;
    }

    // The body of the request.
    inline const std::shared_ptr<ByteContent>& getBody() const {
        return body_;
    }

    inline bool hasBody() const {
        return body_ != nullptr;
    }

    template <typename ValueT = std::shared_ptr<ByteContent>>
    PutObjectRequest& setBody(ValueT&& value) {
        body_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string bucket_;
    std::string key_;
    HeaderCollection metadata_;
    std::shared_ptr<ByteContent> body_;
};

/// The result for the PutObject operation.
class ALIBABACLOUD_OSS_API PutObjectResult final : public ResultModel {
  public:
    PutObjectResult() = default;
    PutObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getHashCrc64ecma() const {
        return getHeaderOrEmpty("x-oss-hash-crc64ecma");
    }

    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the CopyObject operation.
class ALIBABACLOUD_OSS_API CopyObjectRequest final : public RequestModel {
  public:
    CopyObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    CopyObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The path of the source object. By default, this header is left empty.
    // Please use SetSourceBucket, SetSourceKey and SetSourceVersionId instead.
    inline const std::string& getCopySource() const {
        return getHeaderOrEmpty("x-oss-copy-source");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setCopySource(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source", std::forward<ValueT>(value));
        return *this;
    }


    // The name of the source bucket.
    inline const std::string& getSourceBucket() const {
        return sourceBucket_;
    }

    template <typename ValueT = std::string>
    CopyObjectRequest& setSourceBucket(ValueT&& value) {
        sourceBucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the source object.
    inline const std::string& getSourceKey() const {
        return sourceKey_;
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setSourceKey(ValueT&& value) {
        sourceKey_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version id of the source object.
    inline const std::string& getSourceVersionId() const {
        return sourceVersionId_;
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setSourceVersionId(ValueT&& value) {
        sourceVersionId_ = std::forward<ValueT>(value);
        return *this;
    }

    // Specifies whether the CopyObject operation overwrites objects with the same name. The **x-oss-forbid-overwrite**
    // request header does not take effect when versioning is enabled or suspended for the destination bucket. In this
    // case, the CopyObject operation overwrites the existing object that has the same name as the destination object.*
    // If you do not specify the **x-oss-forbid-overwrite** header or set the header to **false**, an existing object
    // that has the same name as the object that you want to copy is overwritten.*****   If you set the
    // **x-oss-forbid-overwrite** header to **true**, an existing object that has the same name as the object that you
    // want to copy is not overwritten.If you specify the **x-oss-forbid-overwrite** header, the queries per second
    // (QPS) performance of OSS may be degraded. If you want to specify the **x-oss-forbid-overwrite** header in a large
    // number of requests (QPS greater than 1,000), contact technical support. Default value: false.
    inline const std::string& getForbidOverwrite() const {
        return getHeaderOrEmpty("x-oss-forbid-overwrite");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setForbidOverwrite(ValueT&& value) {
        headers_.insert_or_assign("x-oss-forbid-overwrite", std::forward<ValueT>(value));
        return *this;
    }

    // The object copy condition. If the ETag value of the source object is the same as the ETag value that you specify
    // in the request, OSS copies the object and returns 200 OK. By default, this header is left empty.
    inline const std::string& getCopySourceIfMatch() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-match");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setCopySourceIfMatch(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-match", std::forward<ValueT>(value));
        return *this;
    }

    // The object copy condition. If the ETag value of the source object is different from the ETag value that you
    // specify in the request, OSS copies the object and returns 200 OK. By default, this header is left empty.
    inline const std::string& getCopySourceIfNoneMatch() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-none-match");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setCopySourceIfNoneMatch(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-none-match", std::forward<ValueT>(value));
        return *this;
    }

    // The object copy condition. If the time that you specify in the request is the same as or later than the
    // modification time of the object, OSS copies the object and returns 200 OK. By default, this header is left empty.
    inline const std::string& getCopySourceIfUnmodifiedSince() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-unmodified-since");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setCopySourceIfUnmodifiedSince(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-unmodified-since", std::forward<ValueT>(value));
        return *this;
    }

    // If the source object is modified after the time that you specify in the request, OSS copies the object. By
    // default, this header is left empty.
    inline const std::string& getCopySourceIfModifiedSince() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-modified-since");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setCopySourceIfModifiedSince(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-modified-since", std::forward<ValueT>(value));
        return *this;
    }

    // The method that is used to configure the metadata of the destination object. Default value: COPY.*   **COPY**:
    // The metadata of the source object is copied to the destination object. The **x-oss-server-side-encryption**
    // attribute of the source object is not copied to the destination object. The **x-oss-server-side-encryption**
    // header in the CopyObject request specifies the method that is used to encrypt the destination object.*
    // **REPLACE**: The metadata that you specify in the request is used as the metadata of the destination object.  If
    // the path of the source object is the same as the path of the destination object and versioning is disabled for
    // the bucket in which the source and destination objects are stored, the metadata that you specify in the
    // CopyObject request is used as the metadata of the destination object regardless of the value of the
    // x-oss-metadata-directive header.
    inline const std::string& getMetadataDirective() const {
        return getHeaderOrEmpty("x-oss-metadata-directive");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setMetadataDirective(ValueT&& value) {
        headers_.insert_or_assign("x-oss-metadata-directive", std::forward<ValueT>(value));
        return *this;
    }

    // The entropy coding-based encryption algorithm that OSS uses to encrypt an object when you create the object. The
    // valid values of the header are **AES256** and **KMS**. You must activate Key Management Service (KMS) in the OSS
    // console before you can use the KMS encryption algorithm. Otherwise, the KmsServiceNotEnabled error is returned.*
    // If you do not specify the **x-oss-server-side-encryption** header in the CopyObject request, the destination
    // object is not encrypted on the server regardless of whether the source object is encrypted on the server.*   If
    // you specify the **x-oss-server-side-encryption** header in the CopyObject request, the destination object is
    // encrypted on the server after the CopyObject operation is performed regardless of whether the source object is
    // encrypted on the server. In addition, the response to a CopyObject request contains the
    // **x-oss-server-side-encryption** header whose value is the encryption algorithm of the destination object. When
    // the destination object is downloaded, the **x-oss-server-side-encryption** header is included in the response.
    // The value of this header is the encryption algorithm of the destination object.
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setServerSideEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The server side data encryption algorithm. Invalid value: SM4
    inline const std::string& getServerSideDataEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-data-encryption");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setServerSideDataEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-data-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The ID of the customer master key (CMK) that is managed by KMS. This parameter is available only if you set
    // **x-oss-server-side-encryption** to KMS.
    inline const std::string& getServerSideEncryptionKeyId() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption-key-id");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setServerSideEncryptionKeyId(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption-key-id", std::forward<ValueT>(value));
        return *this;
    }

    // The access control list (ACL) of the destination object when the object is created. Default value: default.Valid
    // values:*   default: The ACL of the object is the same as the ACL of the bucket in which the object is stored.*
    // private: The ACL of the object is private. Only the owner of the object and authorized users have read and write
    // permissions on the object. Other users do not have permissions on the object.*   public-read: The ACL of the
    // object is public-read. Only the owner of the object and authorized users have read and write permissions on the
    // object. Other users have only read permissions on the object. Exercise caution when you set the ACL of the bucket
    // to this value.*   public-read-write: The ACL of the object is public-read-write. All users have read and write
    // permissions on the object. Exercise caution when you set the ACL of the bucket to this value.For more information
    // about ACLs, see [Object ACL](~~100676~~).
    inline const std::string& getObjectAcl() const {
        return getHeaderOrEmpty("x-oss-object-acl");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setObjectAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-object-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The storage class of the object that you want to upload. Default value: Standard. If you specify a storage class
    // when you upload the object, the storage class applies regardless of the storage class of the bucket to which you
    // upload the object. For example, if you set **x-oss-storage-class** to Standard when you upload an object to an IA
    // bucket, the storage class of the uploaded object is Standard.Valid values:*   Standard*   IA*   Archive*
    // ColdArchiveFor more information about storage classes, see [Overview](~~51374~~).
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setStorageClass(ValueT&& value) {
        headers_.insert_or_assign("x-oss-storage-class", std::forward<ValueT>(value));
        return *this;
    }

    // The tag of the destination object. You can add multiple tags to the destination object. Example: TagA=A\&TagB=B.
    // The tag key and tag value must be URL-encoded. If a key-value pair does not contain an equal sign (=), the tag
    // value is considered an empty string.
    inline const std::string& getTagging() const {
        return getHeaderOrEmpty("x-oss-tagging");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setTagging(ValueT&& value) {
        headers_.insert_or_assign("x-oss-tagging", std::forward<ValueT>(value));
        return *this;
    }

    // The method that is used to add tags to the destination object. Default value: Copy. Valid values:*   **Copy**:
    // The tags of the source object are copied to the destination object.*   **Replace**: The tags that you specify in
    // the request are added to the destination object.
    inline const std::string& getTaggingDirective() const {
        return getHeaderOrEmpty("x-oss-tagging-directive");
    }
    template <typename ValueT = std::string>
    CopyObjectRequest& setTaggingDirective(ValueT&& value) {
        headers_.insert_or_assign("x-oss-tagging-directive", std::forward<ValueT>(value));
        return *this;
    }

    // <no value>
    inline const HeaderCollection& getMetadata() const {
        return metadata_;
    }
    template <typename ValueT = HeaderCollection>
    CopyObjectRequest& setMetadata(ValueT&& value) {
        metadata_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string bucket_;
    std::string key_;
    std::string sourceBucket_;
    std::string sourceKey_;
    std::string sourceVersionId_;
    HeaderCollection metadata_;
};

/*
 * The container that stores the copy result.
 */
struct ALIBABACLOUD_OSS_API CopyObjectResultXml final {
    std::string lastModified;
    std::string eTag;
};

/// The result for the CopyObject operation.
class ALIBABACLOUD_OSS_API CopyObjectResult final : public ResultModel {
  public:
    CopyObjectResult() = default;
    CopyObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    CopyObjectResult(int statusCode, HeaderCollection headers, CopyObjectResultXml body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // <no value>
    inline const std::string& getCopySourceVersionId() const {
        return getHeaderOrEmpty("x-oss-copy-source-version-id");
    }

    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }

    // The ETag value of the destination object.
    inline const std::string& getETag() {
        return body_.eTag;
    }

    // The time when the destination object was last modified.
    inline const std::string& getLastModified() {
        return body_.lastModified;
    }

  private:
    CopyObjectResultXml body_;
};

// The request for the GetObject operation.
class ALIBABACLOUD_OSS_API GetObjectRequest final : public RequestModel {
  public:
    GetObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The range of data of the object to be returned.   - If the value of Range is valid, OSS returns the response that
    // includes the total size of the object and the range of data returned. For example, Content-Range: bytes 0~9/44
    // indicates that the total size of the object is 44 bytes, and the range of data returned is the first 10 bytes. -
    // However, if the value of Range is invalid, the entire object is returned, and the response returned by OSS
    // excludes Content-Range. Default value: null
    inline const std::string& getRange() const {
        return getHeaderOrEmpty("Range");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setRange(ValueT&& value) {
        headers_.insert_or_assign("Range", std::forward<ValueT>(value));
        return *this;
    }

    // If the time specified in this header is earlier than the object modified time or is invalid, OSS returns the
    // object and 200 OK. If the time specified in this header is later than or the same as the object modified time,
    // OSS returns 304 Not Modified. The time must be in GMT. Example: `Fri, 13 Nov 2015 14:47:53 GMT`.Default value:
    // null
    inline const std::string& getIfModifiedSince() const {
        return getHeaderOrEmpty("If-Modified-Since");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setIfModifiedSince(ValueT&& value) {
        headers_.insert_or_assign("If-Modified-Since", std::forward<ValueT>(value));
        return *this;
    }

    // If the time specified in this header is the same as or later than the object modified time, OSS returns the
    // object and 200 OK. If the time specified in this header is earlier than the object modified time, OSS returns 412
    // Precondition Failed.                               The time must be in GMT. Example: `Fri, 13 Nov 2015 14:47:53
    // GMT`.You can specify both the **If-Modified-Since** and **If-Unmodified-Since** headers in a request. Default
    // value: null
    inline const std::string& getIfUnmodifiedSince() const {
        return getHeaderOrEmpty("If-Unmodified-Since");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setIfUnmodifiedSince(ValueT&& value) {
        headers_.insert_or_assign("If-Unmodified-Since", std::forward<ValueT>(value));
        return *this;
    }

    // If the ETag specified in the request matches the ETag value of the object, OSS transmits the object and returns
    // 200 OK. If the ETag specified in the request does not match the ETag value of the object, OSS returns 412
    // Precondition Failed. The ETag value of an object is used to check whether the content of the object has changed.
    // You can check data integrity by using the ETag value. Default value: null
    inline const std::string& getIfMatch() const {
        return getHeaderOrEmpty("If-Match");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setIfMatch(ValueT&& value) {
        headers_.insert_or_assign("If-Match", std::forward<ValueT>(value));
        return *this;
    }

    // If the ETag specified in the request does not match the ETag value of the object, OSS transmits the object and
    // returns 200 OK. If the ETag specified in the request matches the ETag value of the object, OSS returns 304 Not
    // Modified. You can specify both the **If-Match** and **If-None-Match** headers in a request. Default value: null
    inline const std::string& getIfNoneMatch() const {
        return getHeaderOrEmpty("If-None-Match");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setIfNoneMatch(ValueT&& value) {
        headers_.insert_or_assign("If-None-Match", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding type at the client side. If you want an object to be returned in the GZIP format, you must include
    // the Accept-Encoding:gzip header in your request. OSS determines whether to return the object compressed in the
    // GZip format based on the Content-Type header and whether the size of the object is larger than or equal to 1 KB.
    // If an object is compressed in the GZip format, the response OSS returns does not include the ETag value of the
    // object.    - OSS supports the following Content-Type values to compress the object in the GZip format:
    // text/cache-manifest, text/xml, text/plain, text/css, application/javascript, application/x-javascript,
    // application/rss+xml, application/json, and text/json. Default value: null
    inline const std::string& getAcceptEncoding() const {
        return getHeaderOrEmpty("Accept-Encoding");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setAcceptEncoding(ValueT&& value) {
        headers_.insert_or_assign("Accept-Encoding", std::forward<ValueT>(value));
        return *this;
    }

    // The content-type header in the response that OSS returns.
    inline const std::string& getResponseContentType() const {
        return getParameterOrEmpty("response-content-type");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseContentType(ValueT&& value) {
        parameters_.insert_or_assign("response-content-type", std::forward<ValueT>(value));
        return *this;
    }

    // The content-language header in the response that OSS returns.
    inline const std::string& getResponseContentLanguage() const {
        return getParameterOrEmpty("response-content-language");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseContentLanguage(ValueT&& value) {
        parameters_.insert_or_assign("response-content-language", std::forward<ValueT>(value));
        return *this;
    }

    // The expires header in the response that OSS returns.
    inline const std::string& getResponseExpires() const {
        return getParameterOrEmpty("response-expires");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseExpires(ValueT&& value) {
        parameters_.insert_or_assign("response-expires", std::forward<ValueT>(value));
        return *this;
    }

    // The cache-control header in the response that OSS returns.
    inline const std::string& getResponseCacheControl() const {
        return getParameterOrEmpty("response-cache-control");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseCacheControl(ValueT&& value) {
        parameters_.insert_or_assign("response-cache-control", std::forward<ValueT>(value));
        return *this;
    }

    // The content-disposition header in the response that OSS returns.
    inline const std::string& getResponseContentDisposition() const {
        return getParameterOrEmpty("response-content-disposition");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseContentDisposition(ValueT&& value) {
        parameters_.insert_or_assign("response-content-disposition", std::forward<ValueT>(value));
        return *this;
    }

    // The content-encoding header in the response that OSS returns.
    inline const std::string& getResponseContentEncoding() const {
        return getParameterOrEmpty("response-content-encoding");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setResponseContentEncoding(ValueT&& value) {
        parameters_.insert_or_assign("response-content-encoding", std::forward<ValueT>(value));
        return *this;
    }

    // The version ID of the object that you want to query.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    GetObjectRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }

    inline const std::optional<OStreamFactory>& getOStreamFactory() const {
        return ostreamFactory_;
    }

    GetObjectRequest& setOStreamFactory(OStreamFactory value) {
        ostreamFactory_ = std::move(value);
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    std::optional<OStreamFactory> ostreamFactory_;
};

/// The result for the GetObject operation.
class ALIBABACLOUD_OSS_API GetObjectResult final : public ResultModel {
  public:
    GetObjectResult() = default;
    GetObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    GetObjectResult(int statusCode, HeaderCollection headers, std::shared_ptr<std::iostream> body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}


    // <no value>
    inline const std::string& getContentMd5() const {
        return getHeaderOrEmpty("Content-Md5");
    }

    // <no value>
    inline const std::string& getLastModified() const {
        return getHeaderOrEmpty("Last-Modified");
    }

    // <no value>
    inline const std::string& getExpiration() const {
        return getHeaderOrEmpty("x-oss-expiration");
    }

    // <no value>
    inline std::int64_t getTaggingCount() const {
        return getHeaderAsInt64Or("x-oss-tagging-count");
    }

    // <no value>
    inline const std::string& getContentType() const {
        return getHeaderOrEmpty("Content-Type");
    }

    // <no value>
    inline std::int64_t getNextAppendPosition() const {
        return getHeaderAsInt64Or("x-oss-next-append-position");
    }

    // <no value>
    inline const std::string& getHashCrc64ecma() const {
        return getHeaderOrEmpty("x-oss-hash-crc64ecma");
    }

    // <no value>
    inline const std::string& getServerSideEncryptionKeyId() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption-key-id");
    }

    // <no value>
    inline const std::string& getObjectType() const {
        return getHeaderOrEmpty("x-oss-object-type");
    }

    // <no value>
    inline const std::string& getRequestCharged() const {
        return getHeaderOrEmpty("x-oss-request-charged");
    }

    // <no value>
    inline std::int64_t getContentLength() const {
        return getHeaderAsInt64Or("Content-Length");
    }

    // <no value>
    inline const std::string& getETag() const {
        return getHeaderOrEmpty("ETag");
    }

    // <no value>
    inline const HeaderCollection& getMetadata() {
        return metadata_;
    }

    // <no value>
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }

    // <no value>
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }

    // <no value>
    inline const std::string& getRestore() const {
        return getHeaderOrEmpty("x-oss-restore");
    }

    // <no value>
    inline const std::string& getProcessStatus() const {
        return getHeaderOrEmpty("x-oss-process-status");
    }

    // <no value>
    inline const std::shared_ptr<std::iostream>& getBody() {
        return body_;
    }

  private:
    HeaderCollection metadata_;
    std::shared_ptr<std::iostream> body_;
};

// The request for the AppendObject operation.
class ALIBABACLOUD_OSS_API AppendObjectRequest final : public RequestModel {
  public:
    AppendObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    AppendObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The method used to encrypt objects on the specified OSS server. Valid values:- AES256: Keys managed by OSS are
    // used for encryption and decryption (SSE-OSS). - KMS: Keys managed by Key Management Service (KMS) are used for
    // encryption and decryption. - SM4: The SM4 block cipher algorithm is used for encryption and decryption.
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setServerSideEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The access control list (ACL) of the object. Default value: default.  Valid values:- default: The ACL of the
    // object is the same as that of the bucket in which the object is stored. - private: The ACL of the object is
    // private. Only the owner of the object and authorized users can read and write this object. - public-read: The ACL
    // of the object is public-read. Only the owner of the object and authorized users can read and write this object.
    // Other users can only read the object. Exercise caution when you set the object ACL to this value. -
    // public-read-write: The ACL of the object is public-read-write. All users can read and write this object. Exercise
    // caution when you set the object ACL to this value. For more information about the ACL, see [ACL](~~100676~~).
    inline const std::string& getObjectAcl() const {
        return getHeaderOrEmpty("x-oss-object-acl");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setObjectAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-object-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The storage class of the object that you want to upload. Valid values:- Standard- IA- ArchiveIf you specify the
    // object storage class when you upload an object, the storage class of the uploaded object is the specified value
    // regardless of the storage class of the bucket to which the object is uploaded. If you set x-oss-storage-class to
    // Standard when you upload an object to an IA bucket, the object is stored as a Standard object. For more
    // information about storage classes, see the "Overview" topic in Developer Guide. notice The value that you specify
    // takes effect only when you call the AppendObject operation on an object for the first time.
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setStorageClass(ValueT&& value) {
        headers_.insert_or_assign("x-oss-storage-class", std::forward<ValueT>(value));
        return *this;
    }

    // <no value>
    inline const HeaderCollection& getMetadata() const {
        return metadata_;
    }
    template <typename ValueT = HeaderCollection>
    AppendObjectRequest& setMetadata(ValueT&& value) {
        metadata_ = std::forward<ValueT>(value);
        return *this;
    }

    // The web page caching behavior for the object. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getCacheControl() const {
        return getHeaderOrEmpty("Cache-Control");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setCacheControl(ValueT&& value) {
        headers_.insert_or_assign("Cache-Control", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the object when the object is downloaded. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getContentDisposition() const {
        return getHeaderOrEmpty("Content-Disposition");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setContentDisposition(ValueT&& value) {
        headers_.insert_or_assign("Content-Disposition", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding format of the object content. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getContentEncoding() const {
        return getHeaderOrEmpty("Content-Encoding");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setContentEncoding(ValueT&& value) {
        headers_.insert_or_assign("Content-Encoding", std::forward<ValueT>(value));
        return *this;
    }

    // The Content-MD5 header value is a string calculated by using the MD5 algorithm. The header is used to check
    // whether the content of the received message is the same as that of the sent message. To obtain the value of the
    // Content-MD5 header, calculate a 128-bit number based on the message content except for the header, and then
    // encode the number in Base64. Default value: null.Limits: none.
    inline const std::string& getContentMd5() const {
        return getHeaderOrEmpty("Content-MD5");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setContentMd5(ValueT&& value) {
        headers_.insert_or_assign("Content-MD5", std::forward<ValueT>(value));
        return *this;
    }

    // The expiration time. For more information, see **[RFC 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default
    // value: null.
    inline const std::string& getExpires() const {
        return getHeaderOrEmpty("Expires");
    }
    template <typename ValueT = std::string>
    AppendObjectRequest& setExpires(ValueT&& value) {
        headers_.insert_or_assign("Expires", std::forward<ValueT>(value));
        return *this;
    }

    // The position from which the AppendObject operation starts.  Each time an AppendObject operation succeeds, the
    // x-oss-next-append-position header is included in the response to specify the position from which the next
    // AppendObject operation starts. The value of position in the first AppendObject operation performed on an object
    // must be 0. The value of position in subsequent AppendObject operations performed on the object is the current
    // length of the object. For example, if the value of position specified in the first AppendObject request is 0 and
    // the value of content-length is 65536, the value of position in the second AppendObject request must be 65536. -
    // If the value of position in the AppendObject request is 0 and the name of the object that you want to append is
    // unique, you can set headers such as x-oss-server-side-encryption in an AppendObject request in the same way as
    // you set in a PutObject request. If you add the x-oss-server-side-encryption header to an AppendObject request,
    // the x-oss-server-side-encryption header is included in the response to the request. If you want to modify
    // metadata, you can call the CopyObject operation. - If you call an AppendObject operation to append a 0 KB object
    // whose position value is valid to an Appendable object, the status of the Appendable object is not changed.
    inline std::int64_t getPosition() const {
        return getParameterAsInt64Or("position");
    }
    template <typename ValueT = std::int64_t>
    AppendObjectRequest& setPosition(ValueT&& value) {
        parameters_.insert_or_assign("position", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The request body.
    inline const std::shared_ptr<ByteContent>& getBody() const {
        return body_;
    }

    inline bool hasBody() const {
        return body_ != nullptr;
    }

    template <typename ValueT = std::shared_ptr<ByteContent>>
    AppendObjectRequest& setBody(ValueT&& value) {
        body_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    HeaderCollection metadata_;
    std::shared_ptr<ByteContent> body_;
};

/// The result for the AppendObject operation.
class ALIBABACLOUD_OSS_API AppendObjectResult final : public ResultModel {
  public:
    AppendObjectResult() = default;
    AppendObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline std::int64_t getNextAppendPosition() const {
        return getHeaderAsInt64Or("x-oss-next-append-position");
    }

    // <no value>
    inline const std::string& getHashCrc64ecma() const {
        return getHeaderOrEmpty("x-oss-hash-crc64ecma");
    }


  private:
};

// The request for the SealAppendObject operation.
class ALIBABACLOUD_OSS_API SealAppendObjectRequest final : public RequestModel {
  public:
    SealAppendObjectRequest() = default;

    // Bucket name
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    SealAppendObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // Name of the Appendable Object
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    SealAppendObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // Used to specify the expected length of the file when the user wants to seal it.
    inline std::int64_t getPosition() const {
        return getParameterAsInt64Or("position");
    }
    template <typename ValueT = std::int64_t>
    SealAppendObjectRequest& setPosition(ValueT&& value) {
        parameters_.insert_or_assign("position", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the SealAppendObject operation.
class ALIBABACLOUD_OSS_API SealAppendObjectResult final : public ResultModel {
  public:
    SealAppendObjectResult() = default;
    SealAppendObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getSealedTime() const {
        return getHeaderOrEmpty("x-oss-sealed-time");
    }


  private:
};

// The request for the DeleteObject operation.
class ALIBABACLOUD_OSS_API DeleteObjectRequest final : public RequestModel {
  public:
    DeleteObjectRequest() = default;

    // The information about the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    DeleteObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    DeleteObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version ID of the object.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    DeleteObjectRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the DeleteObject operation.
class ALIBABACLOUD_OSS_API DeleteObjectResult final : public ResultModel {
  public:
    DeleteObjectResult() = default;
    DeleteObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getDeleteMarker() const {
        return getHeaderOrEmpty("x-oss-delete-marker");
    }

    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the DeleteMultipleObjects operation.
class ALIBABACLOUD_OSS_API DeleteMultipleObjectsRequest final : public RequestModel {
  public:
    DeleteMultipleObjectsRequest() = default;

    // The information about the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    DeleteMultipleObjectsRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The encoding format of the content in the response.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    DeleteMultipleObjectsRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }

    // The container that stores the request body.
    inline const Delete& getDelete() const {
        return body_.at(0);
    }

    inline bool hasDelete() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = Delete>
    DeleteMultipleObjectsRequest& setDelete(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

  private:
    std::string bucket_;
    std::map<int, Delete> body_;
};

/// The result for the DeleteMultipleObjects operation.
class ALIBABACLOUD_OSS_API DeleteMultipleObjectsResult final : public ResultModel {
  public:
    DeleteMultipleObjectsResult() = default;
    DeleteMultipleObjectsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}

    // The container that stores information about the deleted objects.
    inline const std::vector<DeletedInfo>& getDeletedObjects() const {
        return deletedObjects_;
    }
    template <typename ValueT = std::vector<DeletedInfo>>
    DeleteMultipleObjectsResult& setDeletedObjects(ValueT&& value) {
        deletedObjects_ = std::forward<ValueT>(value);
        return *this;
    }

    // The encoding type of the content in the response.
    inline const std::string& getEncodingType() const {
        return encodingType_;
    }
    template <typename ValueT = std::string>
    DeleteMultipleObjectsResult& setEncodingType(ValueT&& value) {
        encodingType_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::vector<DeletedInfo> deletedObjects_;
    std::string encodingType_;
};


// The request for the HeadObject operation.
class ALIBABACLOUD_OSS_API HeadObjectRequest final : public RequestModel {
  public:
    HeadObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    HeadObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // If the time that is specified in the request is earlier than the time when the object is modified, OSS returns
    // 200 OK and the metadata of the object. Otherwise, OSS returns 304 not modified. Default value: null.
    inline const std::string& getIfModifiedSince() const {
        return getHeaderOrEmpty("If-Modified-Since");
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setIfModifiedSince(ValueT&& value) {
        headers_.insert_or_assign("If-Modified-Since", std::forward<ValueT>(value));
        return *this;
    }

    // If the time that is specified in the request is later than or the same as the time when the object is modified,
    // OSS returns 200 OK and the metadata of the object. Otherwise, OSS returns 412 precondition failed. Default value:
    // null.
    inline const std::string& getIfUnmodifiedSince() const {
        return getHeaderOrEmpty("If-Unmodified-Since");
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setIfUnmodifiedSince(ValueT&& value) {
        headers_.insert_or_assign("If-Unmodified-Since", std::forward<ValueT>(value));
        return *this;
    }

    // If the ETag value that is specified in the request matches the ETag value of the object, OSS returns 200 OK and
    // the metadata of the object. Otherwise, OSS returns 412 precondition failed. Default value: null.
    inline const std::string& getIfMatch() const {
        return getHeaderOrEmpty("If-Match");
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setIfMatch(ValueT&& value) {
        headers_.insert_or_assign("If-Match", std::forward<ValueT>(value));
        return *this;
    }

    // If the ETag value that is specified in the request does not match the ETag value of the object, OSS returns 200
    // OK and the metadata of the object. Otherwise, OSS returns 304 Not Modified. Default value: null.
    inline const std::string& getIfNoneMatch() const {
        return getHeaderOrEmpty("If-None-Match");
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setIfNoneMatch(ValueT&& value) {
        headers_.insert_or_assign("If-None-Match", std::forward<ValueT>(value));
        return *this;
    }

    // The version ID of the object for which you want to query metadata.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    HeadObjectRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the HeadObject operation.
class ALIBABACLOUD_OSS_API HeadObjectResult final : public ResultModel {
  public:
    HeadObjectResult() = default;
    HeadObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getProcessStatus() const {
        return getHeaderOrEmpty("x-oss-process-status");
    }

    // <no value>
    inline const std::string& getRequestCharged() const {
        return getHeaderOrEmpty("x-oss-request-charged");
    }

    // <no value>
    inline const std::string& getContentType() const {
        return getHeaderOrEmpty("Content-Type");
    }

    // <no value>
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }

    // <no value>
    inline const std::string& getObjectType() const {
        return getHeaderOrEmpty("x-oss-object-type");
    }

    // <no value>
    inline const std::string& getExpiration() const {
        return getHeaderOrEmpty("x-oss-expiration");
    }

    // <no value>
    inline const std::string& getContentMd5() const {
        return getHeaderOrEmpty("Content-Md5");
    }

    // <no value>
    inline std::int64_t getContentLength() const {
        return getHeaderAsInt64Or("Content-Length");
    }

    // <no value>
    inline const std::string& getLastModified() const {
        return getHeaderOrEmpty("Last-Modified");
    }

    // <no value>
    inline const std::string& getETag() const {
        return getHeaderOrEmpty("ETag");
    }

    // <no value>
    inline const std::string& getServerSideEncryptionKeyId() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption-key-id");
    }

    // <no value>
    inline std::int64_t getNextAppendPosition() const {
        return getHeaderAsInt64Or("x-oss-next-append-position");
    }

    // <no value>
    inline const std::string& getRestore() const {
        return getHeaderOrEmpty("x-oss-restore");
    }

    // <no value>
    inline const std::string& getTransitionTime() const {
        return getHeaderOrEmpty("x-oss-transition-time");
    }

    // <no value>
    inline std::int64_t getTaggingCount() const {
        return getHeaderAsInt64Or("x-oss-tagging-count");
    }

    // <no value>
    inline const std::string& getHashCrc64ecma() const {
        return getHeaderOrEmpty("x-oss-hash-crc64ecma");
    }

    // <no value>
    inline const HeaderCollection& getMetadata() {
        return metadata_;
    }

    // <no value>
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }

  private:
    HeaderCollection metadata_;
};

// The request for the GetObjectMeta operation.
class ALIBABACLOUD_OSS_API GetObjectMetaRequest final : public RequestModel {
  public:
    GetObjectMetaRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetObjectMetaRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    GetObjectMetaRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The versionID of the object.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    GetObjectMetaRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the GetObjectMeta operation.
class ALIBABACLOUD_OSS_API GetObjectMetaResult final : public ResultModel {
  public:
    GetObjectMetaResult() = default;
    GetObjectMetaResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getLastAccessTime() const {
        return getHeaderOrEmpty("x-oss-last-access-time");
    }

    // <no value>
    inline const std::string& getLastModified() const {
        return getHeaderOrEmpty("Last-Modified");
    }

    // <no value>
    inline const std::string& getTransitionTime() const {
        return getHeaderOrEmpty("x-oss-transition-time");
    }

    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }

    // <no value>
    inline const std::string& getETag() const {
        return getHeaderOrEmpty("ETag");
    }

    // <no value>
    inline std::int64_t getContentLength() const {
        return getHeaderAsInt64Or("Content-Length");
    }


  private:
};

// The request for the RestoreObject operation.
class ALIBABACLOUD_OSS_API RestoreObjectRequest final : public RequestModel {
  public:
    RestoreObjectRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    RestoreObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    RestoreObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version number of the object that you want to restore.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    RestoreObjectRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }

    // The request body schema.
    inline const RestoreRequest& getRestoreRequest() const {
        return body_.at(0);
    }

    inline bool hasRestoreRequest() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = RestoreRequest>
    RestoreObjectRequest& setRestoreRequest(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    std::map<int, RestoreRequest> body_;
};

/// The result for the RestoreObject operation.
class ALIBABACLOUD_OSS_API RestoreObjectResult final : public ResultModel {
  public:
    RestoreObjectResult() = default;
    RestoreObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getObjectRestorePriority() const {
        return getHeaderOrEmpty("x-oss-object-restore-priority");
    }

    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the CleanRestoredObject operation.
class ALIBABACLOUD_OSS_API CleanRestoredObjectRequest final : public RequestModel {
  public:
    CleanRestoredObjectRequest() = default;

    // The name of the bucket
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    CleanRestoredObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    CleanRestoredObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the CleanRestoredObject operation.
class ALIBABACLOUD_OSS_API CleanRestoredObjectResult final : public ResultModel {
  public:
    CleanRestoredObjectResult() = default;
    CleanRestoredObjectResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};


} // namespace models
} // namespace oss2
} // namespace alibabacloud
