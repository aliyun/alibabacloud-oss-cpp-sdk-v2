#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSFwd.h"


#include <memory>
#include <string>
#include <vector>


namespace alibabacloud {
namespace oss2 {

// forward declare
struct ClientConfiguration;

namespace internal {
class ClientImpl;
}

class ALIBABACLOUD_OSS_API OSSClient final {
  public:
    explicit OSSClient(const struct ClientConfiguration& config);

    explicit OSSClient(const struct ClientConfiguration& config, ClientOptionsFns& fns);

    virtual ~OSSClient() = default;

  public:
    /**
     * @brief A generic interface for handling data operations across different types.
     *
     * @param input The input parameter to send
     * @param options Optional, operation options
     * @return OperationResult
     */
    OperationResult invokeOperation(const OperationInput& input, const OperationOptions* options = nullptr);


    /**
     * @brief Queries all buckets that are owned by a requester.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    ListBucketsOutcome listBuckets(const models::ListBucketsRequest& request,
                                   const OperationOptions* options = nullptr);


    /**
     * @brief Queries the endpoints of all supported regions or the endpoints of a specific region.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    DescribeRegionsOutcome describeRegions(const models::DescribeRegionsRequest& request,
                                           const OperationOptions* options = nullptr);

    // Bucket Basic

    /**
     * @brief Queries the storage capacity of a bucket and the number of objects that are stored in the bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetBucketStatOutcome getBucketStat(const models::GetBucketStatRequest& request,
                                       const OperationOptions* options = nullptr);

    /**
     * @brief Creates a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutBucketOutcome putBucket(const models::PutBucketRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief Deletes a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    DeleteBucketOutcome deleteBucket(const models::DeleteBucketRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief Deletes multiple objects from a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    DeleteMultipleObjectsOutcome deleteMultipleObjects(const models::DeleteMultipleObjectsRequest& request,
                                                       const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about all objects in a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    ListObjectsOutcome listObjects(const models::ListObjectsRequest& request,
                                   const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about all objects in a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    ListObjectsV2Outcome listObjectsV2(const models::ListObjectsV2Request& request,
                                       const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about a bucket. Only the owner of a bucket can query the information about the
     * bucket. You can call this operation from an Object Storage Service (OSS) endpoint.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetBucketInfoOutcome getBucketInfo(const models::GetBucketInfoRequest& request,
                                       const OperationOptions* options = nullptr);

    /**
     * @brief Queries the region in which a bucket resides. Only the owner of a bucket can query the region in which the
     * bucket resides.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetBucketLocationOutcome getBucketLocation(const models::GetBucketLocationRequest& request,
                                               const OperationOptions* options = nullptr);


    /**
     * @brief Configures or modifies the access control list (ACL) for a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutBucketAclOutcome putBucketAcl(const models::PutBucketAclRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief Queries the access control list (ACL) of a bucket. Only the owner of a bucket can query the ACL of the
     * bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetBucketAclOutcome getBucketAcl(const models::GetBucketAclRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief Configures a Referer whitelist for an Object Storage Service (OSS) bucket. You can specify whether to
     * allow the requests whose Referer field is empty or whose query strings are truncated.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutBucketRefererOutcome putBucketReferer(const models::PutBucketRefererRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief Queries the hotlink protection configurations for a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetBucketRefererOutcome getBucketReferer(const models::GetBucketRefererRequest& request,
                                             const OperationOptions* options = nullptr);

    // Object Basic
    /**
     * @brief You can call this operation to upload an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutObjectOutcome putObject(const models::PutObjectRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief Copies objects within a bucket or between buckets in the same region.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    CopyObjectOutcome copyObject(const models::CopyObjectRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetObjectOutcome getObject(const models::GetObjectRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to upload an object by appending the object to an existing object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    AppendObjectOutcome appendObject(const models::AppendObjectRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief This operation stops writing to the Appendable Object, after which the user can configure lifecycle rules
     * to change the storage class of the corresponding Appendable Object to Cold Archive or Deep Cold Archive.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    SealAppendObjectOutcome sealAppendObject(const models::SealAppendObjectRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to delete an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    DeleteObjectOutcome deleteObject(const models::DeleteObjectRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the metadata of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    HeadObjectOutcome headObject(const models::HeadObjectRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the metadata of an object, including ETag, Size, and LastModified.
     * The content of the object is not returned.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetObjectMetaOutcome getObjectMeta(const models::GetObjectMetaRequest& request,
                                       const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to restore objects of the Archive and Cold Archive storage classes.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    RestoreObjectOutcome restoreObject(const models::RestoreObjectRequest& request,
                                       const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to clean an object restored from Archive or Cold Archive state. After that,
     * the restored object returns to the frozen state.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    CleanRestoredObjectOutcome cleanRestoredObject(const models::CleanRestoredObjectRequest& request,
                                                   const OperationOptions* options = nullptr);

    // Object Acl
    /**
     * @brief You can call this operation to modify the ACL of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutObjectAclOutcome putObjectAcl(const models::PutObjectAclRequest& request,
                                     const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the ACL of an object in a bucket.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetObjectAclOutcome getObjectAcl(const models::GetObjectAclRequest& request,
                                     const OperationOptions* options = nullptr);

    // Object Symlink

    /**
     * @brief You can create a symbolic link for a target object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutSymlinkOutcome putSymlink(const models::PutSymlinkRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query a symbolic link of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetSymlinkOutcome getSymlink(const models::GetSymlinkRequest& request, const OperationOptions* options = nullptr);


    /**
     * @brief You can call this operation to add tags to or modify the tags of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    PutObjectTaggingOutcome putObjectTagging(const models::PutObjectTaggingRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the tags of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    GetObjectTaggingOutcome getObjectTagging(const models::GetObjectTaggingRequest& request,
                                             const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to delete the tags of a specified object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    DeleteObjectTaggingOutcome deleteObjectTagging(const models::DeleteObjectTaggingRequest& request,
                                                   const OperationOptions* options = nullptr);


    /**
     * @brief Initiates a multipart upload task.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    InitiateMultipartUploadOutcome initiateMultipartUpload(const models::InitiateMultipartUploadRequest& request,
                                                           const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to upload an object by part based on the object name and the upload ID that
     * you specify.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    UploadPartOutcome uploadPart(const models::UploadPartRequest& request, const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to complete the multipart upload task of an object.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    CompleteMultipartUploadOutcome completeMultipartUpload(const models::CompleteMultipartUploadRequest& request,
                                                           const OperationOptions* options = nullptr);

    /**
     * @brief You can call the UploadPartCopy operation by adding the x-oss-copy-source request header to an UploadPart
     * request. This operation copies data from an existing object to upload as a part.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    UploadPartCopyOutcome uploadPartCopy(const models::UploadPartCopyRequest& request,
                                         const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to cancel a multipart upload task and delete the parts that are uploaded by
     * the multipart upload task.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    AbortMultipartUploadOutcome abortMultipartUpload(const models::AbortMultipartUploadRequest& request,
                                                     const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to list all ongoing multipart upload tasks.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    ListMultipartUploadsOutcome listMultipartUploads(const models::ListMultipartUploadsRequest& request,
                                                     const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to list all parts that are uploaded by using a specified upload ID.
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return The result instance
     */
    ListPartsOutcome listParts(const models::ListPartsRequest& request, const OperationOptions* options = nullptr);

    // Presign

    /**
     * @brief Generates a presigned URL for the PutObject operation.
     *
     * @param request The PutObject request to presign
     * @param options Optional, presign options (expiration, etc.)
     * @return PresignOutcome containing the presigned URL and signed headers
     */
    PresignOutcome presign(const models::PutObjectRequest& request,
                           const models::PresignOptions* options = nullptr);

    /**
     * @brief Generates a presigned URL for the GetObject operation.
     *
     * @param request The GetObject request to presign
     * @param options Optional, presign options (expiration, etc.)
     * @return PresignOutcome containing the presigned URL and signed headers
     */
    PresignOutcome presign(const models::GetObjectRequest& request,
                           const models::PresignOptions* options = nullptr);

    /**
     * @brief Generates a presigned URL for the HeadObject operation.
     *
     * @param request The HeadObject request to presign
     * @param options Optional, presign options (expiration, etc.)
     * @return PresignOutcome containing the presigned URL and signed headers
     */
    PresignOutcome presign(const models::HeadObjectRequest& request,
                           const models::PresignOptions* options = nullptr);

    /**
     * @brief Generates a presigned URL for the UploadPart operation.
     *
     * @param request The UploadPart request to presign
     * @param options Optional, presign options (expiration, etc.)
     * @return PresignOutcome containing the presigned URL and signed headers
     */
    PresignOutcome presign(const models::UploadPartRequest& request,
                           const models::PresignOptions* options = nullptr);

  private:
    std::shared_ptr<internal::ClientImpl> client_;
};

} // namespace oss2
} // namespace alibabacloud