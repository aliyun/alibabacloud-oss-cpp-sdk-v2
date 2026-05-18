#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


/*
 * The container that stores the information about multipart upload tasks.
 */
struct ALIBABACLOUD_OSS_API Upload final {
    // The name of the object for which a multipart upload task was initiated.  The results returned by OSS are listed
    // in ascending alphabetical order of object names. Multiple multipart upload tasks that are initiated to upload the
    // same object are listed in ascending order of upload IDs.
    std::string key;

    // The ID of the multipart upload task.
    std::string uploadId;

    // The time when the multipart upload task was initiated.
    std::string initiated;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    Upload& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    Upload& setUploadId(ValueT&& value) {
        uploadId = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    Upload& setInitiated(ValueT&& value) {
        initiated = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the uploaded parts.
 */
struct ALIBABACLOUD_OSS_API Part final {
    // The ETag value that is returned by OSS after the part is uploaded.
    std::string eTag;

    // The part number.
    std::int64_t partNumber;

    // The size of the part.
    std::int64_t size;

    // The time when the part was uploaded.
    std::string lastModified;

    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    Part& setETag(ValueT&& value) {
        eTag = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    Part& setPartNumber(ValueT&& value) {
        partNumber = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::int64_t>
    Part& setSize(ValueT&& value) {
        size = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    Part& setLastModified(ValueT&& value) {
        lastModified = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the content of the CompleteMultipartUpload request.
 */
struct ALIBABACLOUD_OSS_API CompleteMultipartUpload final {
    // The container that stores the uploaded parts.
    std::vector<Part> parts;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<Part>>
    CompleteMultipartUpload& setParts(ValueT&& value) {
        parts = std::forward<ValueT>(value);
        return *this;
    }
};


// The request for the InitiateMultipartUpload operation.
class ALIBABACLOUD_OSS_API InitiateMultipartUploadRequest final : public RequestModel {
  public:
    InitiateMultipartUploadRequest() = default;

    // The name of the bucket to which the object is uploaded by the multipart upload task.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object that is uploaded by the multipart upload task.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // Specifies whether the InitiateMultipartUpload operation overwrites the existing object that has the same name as
    // the object that you want to upload. When versioning is enabled or suspended for the bucket to which you want to
    // upload the object, the **x-oss-forbid-overwrite** header does not take effect. In this case, the
    // InitiateMultipartUpload operation overwrites the existing object that has the same name as the object that you
    // want to upload.   - If you do not specify the **x-oss-forbid-overwrite** header or set the
    // **x-oss-forbid-overwrite** header to **false**, the object that is uploaded by calling the PutObject operation
    // overwrites the existing object that has the same name.   - If the value of **x-oss-forbid-overwrite** is set to
    // **true**, existing objects cannot be overwritten by objects that have the same names. If you specify the
    // **x-oss-forbid-overwrite** request header, the queries per second (QPS) performance of OSS is degraded. If you
    // want to use the **x-oss-forbid-overwrite** request header to perform a large number of operations (QPS greater
    // than 1,000), contact technical support
    inline const std::string& getForbidOverwrite() const {
        return getHeaderOrEmpty("x-oss-forbid-overwrite");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setForbidOverwrite(ValueT&& value) {
        headers_.insert_or_assign("x-oss-forbid-overwrite", std::forward<ValueT>(value));
        return *this;
    }

    // The storage class of the bucket. Default value: Standard.  Valid values:- Standard- IA- Archive- ColdArchive
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setStorageClass(ValueT&& value) {
        headers_.insert_or_assign("x-oss-storage-class", std::forward<ValueT>(value));
        return *this;
    }

    // The tag of the object. You can configure multiple tags for the object. Example: TagA=A&amp;TagB=B. The key and
    // value of a tag must be URL-encoded. If a tag does not contain an equal sign (=), the value of the tag is
    // considered an empty string.
    inline const std::string& getTagging() const {
        return getHeaderOrEmpty("x-oss-tagging");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setTagging(ValueT&& value) {
        headers_.insert_or_assign("x-oss-tagging", std::forward<ValueT>(value));
        return *this;
    }

    // The server-side encryption method that is used to encrypt each part of the object that you want to upload. Valid
    // values: **AES256**, **KMS**, and **SM4**. You must activate Key Management Service (KMS) before you set this
    // header to KMS. If you specify this header in the request, this header is included in the response. OSS uses the
    // method specified by this header to encrypt each uploaded part. When you download the object, the
    // x-oss-server-side-encryption header is included in the response and the header value is set to the algorithm that
    // is used to encrypt the object.
    inline const std::string& getServerSideEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setServerSideEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The algorithm that is used to encrypt the object that you want to upload. If this header is not specified, the
    // object is encrypted by using AES-256. This header is valid only when **x-oss-server-side-encryption** is set to
    // KMS. Valid value: SM4.
    inline const std::string& getServerSideDataEncryption() const {
        return getHeaderOrEmpty("x-oss-server-side-data-encryption");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setServerSideDataEncryption(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-data-encryption", std::forward<ValueT>(value));
        return *this;
    }

    // The ID of the CMK that is managed by KMS. This header is valid only when **x-oss-server-side-encryption** is set
    // to KMS.
    inline const std::string& getServerSideEncryptionKeyId() const {
        return getHeaderOrEmpty("x-oss-server-side-encryption-key-id");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setServerSideEncryptionKeyId(ValueT&& value) {
        headers_.insert_or_assign("x-oss-server-side-encryption-key-id", std::forward<ValueT>(value));
        return *this;
    }

    // The caching behavior of the web page when the object is downloaded. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getCacheControl() const {
        return getHeaderOrEmpty("Cache-Control");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setCacheControl(ValueT&& value) {
        headers_.insert_or_assign("Cache-Control", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the object when the object is downloaded. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getContentDisposition() const {
        return getHeaderOrEmpty("Content-Disposition");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setContentDisposition(ValueT&& value) {
        headers_.insert_or_assign("Content-Disposition", std::forward<ValueT>(value));
        return *this;
    }

    // The content encoding format of the object when the object is downloaded. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getContentEncoding() const {
        return getHeaderOrEmpty("Content-Encoding");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setContentEncoding(ValueT&& value) {
        headers_.insert_or_assign("Content-Encoding", std::forward<ValueT>(value));
        return *this;
    }

    // The expiration time of the request. Unit: milliseconds. For more information, see **[RFC
    // 2616](https://www.ietf.org/rfc/rfc2616.txt)**. Default value: null.
    inline const std::string& getExpires() const {
        return getHeaderOrEmpty("Expires");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setExpires(ValueT&& value) {
        headers_.insert_or_assign("Expires", std::forward<ValueT>(value));
        return *this;
    }

    // The method used to encode the object name in the response. Only URL encoding is supported. The object name can
    // contain characters encoded in UTF-8. However, the XML 1.0 standard cannot be used to parse specific control
    // characters, such as characters whose ASCII values range from 0 to 10. You can configure the encoding-type
    // parameter to encode object names that include characters that cannot be parsed by XML 1.0 in the
    // response.brDefault value: null
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    InitiateMultipartUploadRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/*
 * The container that stores the result of InitiateMultipartUpload request.
 */
struct ALIBABACLOUD_OSS_API InitiateMultipartUploadResultXml final {
    std::string bucket;
    std::string key;
    std::string uploadId;
    std::string encodingType;
};


/// The result for the InitiateMultipartUpload operation.
class ALIBABACLOUD_OSS_API InitiateMultipartUploadResult final : public ResultModel {
  public:
    InitiateMultipartUploadResult() = default;
    InitiateMultipartUploadResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}

    InitiateMultipartUploadResult(int statusCode, HeaderCollection headers, InitiateMultipartUploadResultXml body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // The name of the object that is uploaded by the multipart upload task.
    inline const std::string& getKey() {
        return body_.key;
    }

    // The upload ID that uniquely identifies the multipart upload task. The upload ID is used to call UploadPart and
    // CompleteMultipartUpload later.
    inline const std::string& getUploadId() {
        return body_.uploadId;
    }

    // The encoding type of the object name in the response. If the encoding-type parameter is specified in the request,
    // the object name in the response is encoded.
    inline const std::string& getEncodingType() {
        return body_.encodingType;
    }

    // The name of the bucket to which the object is uploaded by the multipart upload task.
    inline const std::string& getBucket() {
        return body_.bucket;
    }

  private:
    InitiateMultipartUploadResultXml body_;
};

// The request for the UploadPart operation.
class ALIBABACLOUD_OSS_API UploadPartRequest final : public RequestModel {
  public:
    UploadPartRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    UploadPartRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    UploadPartRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The number that identifies a part. Valid values: 1 to 10000.The size of a part ranges from 100 KB to 5 GB.  In
    // multipart upload, each part except the last part must be larger than or equal to 100 KB in size. When you call
    // the UploadPart operation, the size of each part is not verified because not all parts have been uploaded and OSS
    // does not know which part is the last part. The size of each part is verified only when you call
    // CompleteMultipartUpload.
    inline std::int64_t getPartNumber() const {
        return getParameterAsInt64Or("partNumber");
    }
    template <typename ValueT = std::int64_t>
    UploadPartRequest& setPartNumber(ValueT&& value) {
        parameters_.insert_or_assign("partNumber", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The ID that identifies the object to which the part that you want to upload belongs.
    inline const std::string& getUploadId() const {
        return getParameterOrEmpty("uploadId");
    }
    template <typename ValueT = std::string>
    UploadPartRequest& setUploadId(ValueT&& value) {
        parameters_.insert_or_assign("uploadId", std::forward<ValueT>(value));
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
    UploadPartRequest& setBody(ValueT&& value) {
        body_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<ProgressCallback>& getProgressCallback() const {
        return progressCallback_;
    }
    template <typename ValueT = ProgressCallback>
    UploadPartRequest& setProgressCallback(ValueT&& value) {
        progressCallback_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string bucket_;
    std::string key_;
    std::shared_ptr<ByteContent> body_;
    std::optional<ProgressCallback> progressCallback_;
};

/// The result for the UploadPart operation.
class ALIBABACLOUD_OSS_API UploadPartResult final : public ResultModel {
  public:
    UploadPartResult() = default;
    UploadPartResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    inline const std::string& getETag() const {
        return getHeaderOrEmpty("ETag");
    }

    inline const std::string& getHashCrc64ecma() const {
        return getHeaderOrEmpty("x-oss-hash-crc64ecma");
    }

  private:
};

// The request for the CompleteMultipartUpload operation.
class ALIBABACLOUD_OSS_API CompleteMultipartUploadRequest final : public RequestModel {
  public:
    CompleteMultipartUploadRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // Specifieswhethertheobjectwith the sameobjectname is overwritten when you call the CompleteMultipartUpload
    // operation.- If the value of x-oss-forbid-overwrite is not specified or set to false, the existing object can be
    // overwritten by the object that has the same name. - If the value of x-oss-forbid-overwrite is set to true, the
    // existing object cannot be overwritten by the object that has the same name. - The x-oss-forbid-overwrite request
    // header is invalid if versioning is enabled or suspended for the bucket. In this case, the existing object can be
    // overwritten by the object that has the same name when you call the CompleteMultipartUpload operation. - If you
    // specify the x-oss-forbid-overwrite request header, the queries per second (QPS) performance of OSS may be
    // degraded. If you want to configure the x-oss-forbid-overwrite header in a large number of requests (QPS  1,000),
    // submit a ticket.
    inline const std::string& getForbidOverwrite() const {
        return getHeaderOrEmpty("x-oss-forbid-overwrite");
    }
    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setForbidOverwrite(ValueT&& value) {
        headers_.insert_or_assign("x-oss-forbid-overwrite", std::forward<ValueT>(value));
        return *this;
    }

    // Specifies whether to list all parts that are uploaded by using the current upload ID.Valid value: yes.- If
    // x-oss-complete-all is set to yes in the request, OSS lists all parts that are uploaded by using the current
    // upload ID, sorts the parts by part number, and then performs the CompleteMultipartUpload operation. When OSS
    // performs the CompleteMultipartUpload operation, OSS cannot detect the parts that are not uploaded or currently
    // being uploaded. Before you call the CompleteMultipartUpload operation, make sure that all parts are uploaded.- If
    // x-oss-complete-all is specified in the request, the request body cannot be specified. Otherwise, an error
    // occurs.- If x-oss-complete-all is specified in the request, the format of the response remains unchanged.
    inline const std::string& getCompleteAll() const {
        return getHeaderOrEmpty("x-oss-complete-all");
    }
    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setCompleteAll(ValueT&& value) {
        headers_.insert_or_assign("x-oss-complete-all", std::forward<ValueT>(value));
        return *this;
    }

    // The identifier of the multipart upload task.
    inline const std::string& getUploadId() const {
        return getParameterOrEmpty("uploadId");
    }
    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setUploadId(ValueT&& value) {
        parameters_.insert_or_assign("uploadId", std::forward<ValueT>(value));
        return *this;
    }

    // The encodingtype of the object name in the response. Only URL encoding is supported.The object name can contain
    // characters that are encoded in UTF-8. However, the XML 1.0 standard cannot be used to parse control characters,
    // such as characters with an ASCII value from 0 to 10. You can configure this parameter to encode the object name
    // in the response.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    CompleteMultipartUploadRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }

    // The request body schema.
    inline const CompleteMultipartUpload& getCompleteMultipartUpload() const {
        return body_.at(0);
    }

    inline bool hasCompleteMultipartUpload() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = CompleteMultipartUpload>
    CompleteMultipartUploadRequest& setCompleteMultipartUpload(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    std::map<int, CompleteMultipartUpload> body_;
};


/// The result for the CompleteMultipartUpload operation.
class ALIBABACLOUD_OSS_API CompleteMultipartUploadResult final : public ResultModel {
  public:
    CompleteMultipartUploadResult() = default;
    CompleteMultipartUploadResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }

    // The URL that is used to access the uploaded object.
    inline const std::string& getLocation() {
        return location_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadResult& setLocation(ValueT&& value) {
        location_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the bucket that contains the object you want to restore.
    inline const std::string& getBucket() {
        return bucket_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadResult& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the uploaded object.
    inline const std::string& getKey() {
        return key_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadResult& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The ETag that is generated when an object is created. ETags are used to identify the content of objects.If an
    // object is created by calling the CompleteMultipartUpload operation, the ETag value is not the MD5 hash of the
    // object content but a unique value calculated based on a specific rule. The ETag of an object can be used to check
    // whether the object content is modified. However, we recommend that you use the MD5 hash of an object rather than
    // the ETag value of the object to verify data integrity.
    inline const std::string& getETag() {
        return eTag_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadResult& setETag(ValueT&& value) {
        eTag_ = std::forward<ValueT>(value);
        return *this;
    }

    // The encoding type of the object name in the response. If this parameter is specified in the request, the object
    // name is encoded in the response.
    inline const std::string& getEncodingType() {
        return encodingType_;
    }

    template <typename ValueT = std::string>
    CompleteMultipartUploadResult& setEncodingType(ValueT&& value) {
        encodingType_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string location_;
    std::string bucket_;
    std::string key_;
    std::string eTag_;
    std::string encodingType_;
};

// The request for the UploadPartCopy operation.
class ALIBABACLOUD_OSS_API UploadPartCopyRequest final : public RequestModel {
  public:
    UploadPartCopyRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    UploadPartCopyRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The address to access the source object. You must have permissions to read the source object.
    // Please use SetSourceBucket, SetSourceKey and SetSourceVersionId instead.
    inline const std::string& getCopySource() const {
        return getHeaderOrEmpty("x-oss-copy-source");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySource(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the source bucket.
    inline const std::string& getSourceBucket() const {
        return sourceBucket_;
    }

    template <typename ValueT = std::string>
    UploadPartCopyRequest& setSourceBucket(ValueT&& value) {
        sourceBucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the source object.
    inline const std::string& getSourceKey() const {
        return sourceKey_;
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setSourceKey(ValueT&& value) {
        sourceKey_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version id of the source object.
    inline const std::string& getSourceVersionId() const {
        return sourceVersionId_;
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setSourceVersionId(ValueT&& value) {
        sourceVersionId_ = std::forward<ValueT>(value);
        return *this;
    }

    // The range of bytes to copy data from the source object. For example, if you specify bytes to 0 to 9, the system
    // transfers byte 0 to byte 9, a total of 10 bytes.brDefault value: null- If the x-oss-copy-source-range request
    // header is not specified, the entire source object is copied.- If the x-oss-copy-source-range request header is
    // specified, the response contains the length of the entire object and the range of bytes to be copied for this
    // operation. For example, Content-Range: bytes 0~9/44 indicates that the length of the entire object is 44 bytes.
    // The range of bytes to be copied is byte 0 to byte 9.- If the specified range does not conform to the range
    // conventions, OSS copies the entire object and does not include Content-Range in the response.
    inline const std::string& getCopySourceRange() const {
        return getHeaderOrEmpty("x-oss-copy-source-range");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySourceRange(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-range", std::forward<ValueT>(value));
        return *this;
    }

    // The copy operation condition. If the ETag value of the source object is the same as the ETag value provided by
    // the user, OSS copies data. Otherwise, OSS returns 412 Precondition Failed.brDefault value: null
    inline const std::string& getCopySourceIfMatch() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-match");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySourceIfMatch(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-match", std::forward<ValueT>(value));
        return *this;
    }

    // The object transfer condition. If the input ETag value does not match the ETag value of the object, the system
    // transfers the object normally and returns 200 OK. Otherwise, OSS returns 304 Not Modified.brDefault value: null
    inline const std::string& getCopySourceIfNoneMatch() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-none-match");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySourceIfNoneMatch(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-none-match", std::forward<ValueT>(value));
        return *this;
    }

    // The object transfer condition. If the specified time is the same as or later than the actual modified time of the
    // object, OSS transfers the object normally and returns 200 OK. Otherwise, OSS returns 412 Precondition
    // Failed.brDefault value: null
    inline const std::string& getCopySourceIfUnmodifiedSince() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-unmodified-since");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySourceIfUnmodifiedSince(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-unmodified-since", std::forward<ValueT>(value));
        return *this;
    }

    // The object transfer condition. If the specified time is earlier than the actual modified time of the object, the
    // system transfers the object normally and returns 200 OK. Otherwise, OSS returns 304 Not Modified.brDefault value:
    // nullbrTime format: ddd, dd MMM yyyy HH:mm:ss GMT. Example: Fri, 13 Nov 2015 14:47:53 GMT.
    inline const std::string& getCopySourceIfModifiedSince() const {
        return getHeaderOrEmpty("x-oss-copy-source-if-modified-since");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setCopySourceIfModifiedSince(ValueT&& value) {
        headers_.insert_or_assign("x-oss-copy-source-if-modified-since", std::forward<ValueT>(value));
        return *this;
    }

    // The number of parts.
    inline std::int64_t getPartNumber() const {
        return getParameterAsInt64Or("partNumber");
    }
    template <typename ValueT = std::int64_t>
    UploadPartCopyRequest& setPartNumber(ValueT&& value) {
        parameters_.insert_or_assign("partNumber", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The ID that identifies the object to which the parts to upload belong.
    inline const std::string& getUploadId() const {
        return getParameterOrEmpty("uploadId");
    }
    template <typename ValueT = std::string>
    UploadPartCopyRequest& setUploadId(ValueT&& value) {
        parameters_.insert_or_assign("uploadId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    std::string sourceBucket_;
    std::string sourceKey_;
    std::string sourceVersionId_;
};

/*
 * The container that stores the copy result.
 */
struct ALIBABACLOUD_OSS_API CopyPartResult final {
    std::string lastModified;
    std::string eTag;
};

/// The result for the UploadPartCopy operation.
class ALIBABACLOUD_OSS_API UploadPartCopyResult final : public ResultModel {
  public:
    UploadPartCopyResult() = default;
    UploadPartCopyResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    UploadPartCopyResult(int statusCode, HeaderCollection headers, CopyPartResult body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // <no value>
    inline const std::string& getCopySourceVersionId() const {
        return getHeaderOrEmpty("x-oss-copy-source-version-id");
    }

    // The ETag of the copied part.
    inline const std::string& getETag() const {
        return body_.eTag;
    }

    // The last modified time of copy source.
    inline const std::string& getLastModified() const {
        return body_.lastModified;
    }

  private:
    CopyPartResult body_;
};

// The request for the AbortMultipartUpload operation.
class ALIBABACLOUD_OSS_API AbortMultipartUploadRequest final : public RequestModel {
  public:
    AbortMultipartUploadRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    AbortMultipartUploadRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object that you want to upload.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    AbortMultipartUploadRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The ID of the multipart upload task.
    inline const std::string& getUploadId() const {
        return getParameterOrEmpty("uploadId");
    }
    template <typename ValueT = std::string>
    AbortMultipartUploadRequest& setUploadId(ValueT&& value) {
        parameters_.insert_or_assign("uploadId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the AbortMultipartUpload operation.
class ALIBABACLOUD_OSS_API AbortMultipartUploadResult final : public ResultModel {
  public:
    AbortMultipartUploadResult() = default;
    AbortMultipartUploadResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the ListMultipartUploads operation.
class ALIBABACLOUD_OSS_API ListMultipartUploadsRequest final : public RequestModel {
  public:
    ListMultipartUploadsRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The character used to group objects by name. Objects whose names contain the same string that ranges from the
    // specified prefix to the delimiter that appears for the first time are grouped as a CommonPrefixes element.
    inline const std::string& getDelimiter() const {
        return getParameterOrEmpty("delimiter");
    }
    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setDelimiter(ValueT&& value) {
        parameters_.insert_or_assign("delimiter", std::forward<ValueT>(value));
        return *this;
    }

    // The maximumnumber of multipart upload tasks that can be returned for the current request. Default value: 1000.
    // Maximum value: 1000.
    inline std::int64_t getMaxUploads() const {
        return getParameterAsInt64Or("max-uploads");
    }
    template <typename ValueT = std::int64_t>
    ListMultipartUploadsRequest& setMaxUploads(ValueT&& value) {
        parameters_.insert_or_assign("max-uploads", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // This parameter is used together with the upload-id-marker parameter to specify the position from which the next
    // list begins.- If the upload-id-marker parameter is not set, Object Storage Service (OSS) returns all multipart
    // upload tasks in which object names are alphabetically after the key-marker value.- If the upload-id-marker
    // parameter is set, the response includes the following tasks:  - Multipart upload tasks in which object names are
    // alphabetically after the key-marker value in alphabetical order  - Multipart upload tasks in which object names
    // are the same as the key-marker parameter value but whose upload IDs are greater than the upload-id-marker
    // parameter value
    inline const std::string& getKeyMarker() const {
        return getParameterOrEmpty("key-marker");
    }
    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setKeyMarker(ValueT&& value) {
        parameters_.insert_or_assign("key-marker", std::forward<ValueT>(value));
        return *this;
    }

    // The prefix that the returned object names must contain. If you specify a prefix in the request, the specified
    // prefix is included in the response.You can use prefixes to group and manage objects in buckets in the same way
    // you manage a folder in a file system.
    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    // The upload ID of the multipart upload task after which the list begins. This parameter is used together with the
    // key-marker parameter.- If the key-marker parameter is not set, OSS ignores the upload-id-marker parameter.- If
    // the key-marker parameter is configured, the query result includes:  - Multipart upload tasks in which object
    // names are alphabetically after the key-marker value in alphabetical order  - Multipart upload tasks in which
    // object names are the same as the key-marker parameter value but whose upload IDs are greater than the
    // upload-id-marker parameter value
    inline const std::string& getUploadIdMarker() const {
        return getParameterOrEmpty("upload-id-marker");
    }
    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setUploadIdMarker(ValueT&& value) {
        parameters_.insert_or_assign("upload-id-marker", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding type of the object name in the response. Values of Delimiter, KeyMarker, Prefix, NextKeyMarker, and
    // Key can be encoded in UTF-8. However, the XML 1.0 standard cannot be used to parse control characters such as
    // characters with an American Standard Code for Information Interchange (ASCII) value from 0 to 10. You can set the
    // encoding-type parameter to encode values of Delimiter, KeyMarker, Prefix, NextKeyMarker, and Key in the
    // response.Default value: null
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    ListMultipartUploadsRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
};

/*
 * The container that stores the result of ListMultipartUploads request.
 */
struct ALIBABACLOUD_OSS_API ListMultipartUploadsResultXml final {
    std::string bucket;
    std::string keyMarker;
    std::string uploadIdMarker;
    std::string nextKeyMarker;
    std::string nextUploadIdMarker;
    std::string delimiter;
    std::string prefix;
    std::optional<std::int64_t> maxUploads;
    std::optional<bool> isTruncated;
    std::string encodingType;
    std::vector<Upload> uploads;
    std::vector<CommonPrefix> commonPrefixes;
};

/// The result for the ListMultipartUploads operation.
class ALIBABACLOUD_OSS_API ListMultipartUploadsResult final : public ResultModel {
  public:
    ListMultipartUploadsResult() = default;
    ListMultipartUploadsResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}

    ListMultipartUploadsResult(int statusCode, HeaderCollection headers, ListMultipartUploadsResultXml body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // The prefix that the returned object names must contain. If you specify a prefix in the request, the specified
    // prefix is included in the response.
    inline const std::string& getPrefix() const {
        return body_.prefix;
    }

    // The character used to group objects by name. If you specify the Delimiter parameter in the request, the response
    // contains the CommonPrefixes element. Objects whose names contain the same string from the prefix to the next
    // occurrence of the delimiter are grouped as a single result element in
    inline const std::string& getDelimiter() const {
        return body_.delimiter;
    }

    // The ID list of the multipart upload tasks.
    inline const std::vector<Upload>& getUploads() const {
        return body_.uploads;
    }

    // The name of the object that corresponds to the multipart upload task after which the list begins.
    inline const std::string& getKeyMarker() const {
        return body_.keyMarker;
    }

    // The upload ID of the multipart upload task after which the list begins.
    inline const std::string& getUploadIdMarker() const {
        return body_.uploadIdMarker;
    }

    // The NextUploadMarker value that is used for the UploadMarker value in the next request if the response does not
    // contain all required results.
    inline const std::string& getNextUploadIdMarker() const {
        return body_.nextUploadIdMarker;
    }

    // If the delimiter parameter is specified in the request, the response contains the CommonPrefixes parameter. The
    // objects whose names contain the same string from the prefix to the next occurrence of the delimiter are grouped
    // as a single result element in the CommonPrefixes parameter.
    inline const std::vector<CommonPrefix>& getCommonPrefixes() const {
        return body_.commonPrefixes;
    }

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return body_.bucket;
    }

    // The method used to encode the object name in the response. If encoding-type is specified in the request, values
    // of those elements including Delimiter, KeyMarker, Prefix, NextKeyMarker, and Key are encoded in the returned
    // result.
    inline const std::string& getEncodingType() const {
        return body_.encodingType;
    }

    // The object name marker in the response for the next request to return the remaining results.
    inline const std::string& getNextKeyMarker() {
        return body_.nextKeyMarker;
    }

    // The maximum number of multipart upload tasks returned by OSS.
    // -1 means the value is unkonwn.
    inline std::int64_t getMaxUploads() {
        return body_.maxUploads.value_or(-1);
    }

    // Indicates whether the list of multipart upload tasks returned in the response is truncated. Default value: false.
    // Valid values:- true: Only part of the results are returned this time.- false: All results are returned.
    inline bool getIsTruncated() {
        return body_.isTruncated.value_or(false);
    }

  private:
    ListMultipartUploadsResultXml body_;
};

// The request for the ListParts operation.
class ALIBABACLOUD_OSS_API ListPartsRequest final : public RequestModel {
  public:
    ListPartsRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    ListPartsRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    ListPartsRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The ID of the multipart upload task.By default, this parameter is left empty.
    inline const std::string& getUploadId() const {
        return getParameterOrEmpty("uploadId");
    }
    template <typename ValueT = std::string>
    ListPartsRequest& setUploadId(ValueT&& value) {
        parameters_.insert_or_assign("uploadId", std::forward<ValueT>(value));
        return *this;
    }

    // The maximum number of parts that can be returned by OSS.Default value: 1000.Maximum value: 1000.
    inline std::int64_t getMaxParts() const {
        return getParameterAsInt64Or("max-parts");
    }
    template <typename ValueT = std::int64_t>
    ListPartsRequest& setMaxParts(ValueT&& value) {
        parameters_.insert_or_assign("max-parts", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The position from which the list starts. All parts whose part numbers are greater than the value of this
    // parameter are listed.By default, this parameter is left empty.
    inline std::int64_t getPartNumberMarker() const {
        return getParameterAsInt64Or("part-number-marker");
    }
    template <typename ValueT = std::int64_t>
    ListPartsRequest& setPartNumberMarker(ValueT&& value) {
        parameters_.insert_or_assign("part-number-marker", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The maximum number of parts that can be returned by OSS. Default value: 1000.Maximum value: 1000.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    ListPartsRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};


/*
 * The container that stores the response of the ListParts request.
 */
struct ALIBABACLOUD_OSS_API ListPartResultXml final {
    std::string bucket;
    std::string key;
    std::string uploadId;
    std::string encodingType;
    std::optional<std::int64_t> partNumberMarker;
    std::optional<std::int64_t> nextPartNumberMarker;
    std::optional<std::int64_t> maxParts;
    std::optional<bool> isTruncated;
    std::vector<Part> parts;
};

/// The result for the ListParts operation.
class ALIBABACLOUD_OSS_API ListPartsResult final : public ResultModel {
  public:
    ListPartsResult() = default;
    ListPartsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    ListPartsResult(int statusCode, HeaderCollection headers, ListPartResultXml body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // The name of the object.
    inline const std::string& getBucket() const {
        return body_.bucket;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return body_.key;
    }

    // The ID of the upload task.
    inline const std::string& getUploadId() const {
        return body_.uploadId;
    }

    // The position from which the list starts. All parts whose part numbers are greater than the value of this
    // parameter are listed.
    inline std::int64_t getPartNumberMarker() const {
        return body_.partNumberMarker.value_or(-1);
    }

    // The NextPartNumberMarker value that is used for the PartNumberMarker value in a subsequent request when the
    // response does not contain all required results.
    inline std::int64_t getNextPartNumberMarker() const {
        return body_.nextPartNumberMarker.value_or(-1);
    }

    // The maximum number of parts in the response.
    inline std::int64_t getMaxParts() const {
        return body_.maxParts.value_or(-1);
    }

    // Indicates whether the list of parts returned in the response has been truncated. A value of true indicates that
    // the response does not contain all required results. A value of false indicates that the response contains all
    // required results.Valid values: true and false.
    inline bool getIsTruncated() const {
        return body_.isTruncated.value_or(false);
    }

    // The list of all parts.
    inline const std::vector<Part>& getParts() const {
        return body_.parts;
    }

    // The encoding type of the object name in the response. If the encoding-type parameter is specified in the request,
    // the object name in the response is encoded.
    inline const std::string& getEncodingType() {
        return body_.encodingType;
    }

  private:
    ListPartResultXml body_;
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
