#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSFwd.h"

#include <future>
#include <memory>

namespace alibabacloud {
namespace oss2 {

struct ClientConfiguration;

namespace internal {
class AsyncClientImpl;
}

class ALIBABACLOUD_OSS_API OSSAsyncClient final {
  public:
    explicit OSSAsyncClient(const struct ClientConfiguration& config);
    explicit OSSAsyncClient(const struct ClientConfiguration& config, ClientOptionsFns& fns);
    ~OSSAsyncClient();

    void invokeOperationAsync(const OperationInput& input,
                              const OperationCallback& callback,
                              const OperationOptions* options = nullptr);

    // Service

    /**
     * @brief Queries all buckets that are owned by a requester.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listBucketsAsync(const models::ListBucketsRequest& request,
                          const ListBucketsAsyncCallback& callback,
                          const OperationOptions* options = nullptr);

    // Region

    /**
     * @brief Queries the endpoints of all supported regions or the endpoints of a specific region, including the
     * internal endpoints and the endpoints that are used to accelerate data transfer.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void describeRegionsAsync(const models::DescribeRegionsRequest& request,
                              const DescribeRegionsAsyncCallback& callback,
                              const OperationOptions* options = nullptr);

    // Bucket Basic

    /**
     * @brief Queries the storage capacity of a bucket and the number of objects that are stored in the bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketStatAsync(const models::GetBucketStatRequest& request,
                            const GetBucketStatAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief Creates a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putBucketAsync(const models::PutBucketRequest& request,
                        const PutBucketAsyncCallback& callback,
                        const OperationOptions* options = nullptr);

    /**
     * @brief Deletes a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void deleteBucketAsync(const models::DeleteBucketRequest& request,
                           const DeleteBucketAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about all objects in a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listObjectsAsync(const models::ListObjectsRequest& request,
                          const ListObjectsAsyncCallback& callback,
                          const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about all objects in a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listObjectsV2Async(const models::ListObjectsV2Request& request,
                            const ListObjectsV2AsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief Queries the information about a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketInfoAsync(const models::GetBucketInfoRequest& request,
                            const GetBucketInfoAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief Queries the region in which a bucket resides.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketLocationAsync(const models::GetBucketLocationRequest& request,
                                const GetBucketLocationAsyncCallback& callback,
                                const OperationOptions* options = nullptr);

    // Bucket Acl

    /**
     * @brief Configures or modifies the access control list (ACL) for a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putBucketAclAsync(const models::PutBucketAclRequest& request,
                           const PutBucketAclAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief Queries the access control list (ACL) of a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketAclAsync(const models::GetBucketAclRequest& request,
                           const GetBucketAclAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    // Bucket Referer

    /**
     * @brief Configures a Referer whitelist for an Object Storage Service (OSS) bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putBucketRefererAsync(const models::PutBucketRefererRequest& request,
                               const PutBucketRefererAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    /**
     * @brief Queries the hotlink protection configurations for a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketRefererAsync(const models::GetBucketRefererRequest& request,
                               const GetBucketRefererAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    // Object Basic

    /**
     * @brief You can call this operation to upload an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putObjectAsync(const models::PutObjectRequest& request,
                        const PutObjectAsyncCallback& callback,
                        const OperationOptions* options = nullptr);

    /**
     * @brief Copies objects within a bucket or between buckets in the same region.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void copyObjectAsync(const models::CopyObjectRequest& request,
                         const CopyObjectAsyncCallback& callback,
                         const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getObjectAsync(const models::GetObjectRequest& request,
                        const GetObjectAsyncCallback& callback,
                        const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to upload an object by appending the object to an existing object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void appendObjectAsync(const models::AppendObjectRequest& request,
                           const AppendObjectAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief This operation stops writing to the Appendable Object and seals it.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void sealAppendObjectAsync(const models::SealAppendObjectRequest& request,
                               const SealAppendObjectAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to delete an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void deleteObjectAsync(const models::DeleteObjectRequest& request,
                           const DeleteObjectAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief Deletes multiple objects from a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void deleteMultipleObjectsAsync(const models::DeleteMultipleObjectsRequest& request,
                                    const DeleteMultipleObjectsAsyncCallback& callback,
                                    const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the metadata of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void headObjectAsync(const models::HeadObjectRequest& request,
                         const HeadObjectAsyncCallback& callback,
                         const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the metadata of an object, including ETag, Size, and
     * LastModified.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getObjectMetaAsync(const models::GetObjectMetaRequest& request,
                            const GetObjectMetaAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to restore objects of the Archive or Cold Archive storage class.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void restoreObjectAsync(const models::RestoreObjectRequest& request,
                            const RestoreObjectAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to clean an object restored from the Archive or Cold Archive storage
     * class.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void cleanRestoredObjectAsync(const models::CleanRestoredObjectRequest& request,
                                  const CleanRestoredObjectAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    // Object Acl

    /**
     * @brief You can call this operation to modify the ACL of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putObjectAclAsync(const models::PutObjectAclRequest& request,
                           const PutObjectAclAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the ACL of an object in a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getObjectAclAsync(const models::GetObjectAclRequest& request,
                           const GetObjectAclAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    // Object Symlink

    /**
     * @brief You can create a symbolic link for a target object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putSymlinkAsync(const models::PutSymlinkRequest& request,
                         const PutSymlinkAsyncCallback& callback,
                         const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query a symbolic link of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getSymlinkAsync(const models::GetSymlinkRequest& request,
                         const GetSymlinkAsyncCallback& callback,
                         const OperationOptions* options = nullptr);

    // Object Tagging

    /**
     * @brief You can call this operation to add tags to or modify the tags of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putObjectTaggingAsync(const models::PutObjectTaggingRequest& request,
                               const PutObjectTaggingAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to query the tags of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getObjectTaggingAsync(const models::GetObjectTaggingRequest& request,
                               const GetObjectTaggingAsyncCallback& callback,
                               const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to delete the tags of a specified object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void deleteObjectTaggingAsync(const models::DeleteObjectTaggingRequest& request,
                                  const DeleteObjectTaggingAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    // Object Multipart

    /**
     * @brief Initiates a multipart upload task.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void initiateMultipartUploadAsync(const models::InitiateMultipartUploadRequest& request,
                                      const InitiateMultipartUploadAsyncCallback& callback,
                                      const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to upload an object by part based on the object name and the upload ID
     * that you specify.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void uploadPartAsync(const models::UploadPartRequest& request,
                         const UploadPartAsyncCallback& callback,
                         const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to complete the multipart upload task of an object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void completeMultipartUploadAsync(const models::CompleteMultipartUploadRequest& request,
                                      const CompleteMultipartUploadAsyncCallback& callback,
                                      const OperationOptions* options = nullptr);

    /**
     * @brief You can call the UploadPartCopy operation by adding the x-oss-copy-source header to upload a part
     * by copying data from an existing object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void uploadPartCopyAsync(const models::UploadPartCopyRequest& request,
                             const UploadPartCopyAsyncCallback& callback,
                             const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to cancel a multipart upload task and delete the parts uploaded in the
     * task.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void abortMultipartUploadAsync(const models::AbortMultipartUploadRequest& request,
                                   const AbortMultipartUploadAsyncCallback& callback,
                                   const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to list all ongoing multipart upload tasks.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listMultipartUploadsAsync(const models::ListMultipartUploadsRequest& request,
                                   const ListMultipartUploadsAsyncCallback& callback,
                                   const OperationOptions* options = nullptr);

    /**
     * @brief You can call this operation to list all parts that are uploaded by using a specified upload ID.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listPartsAsync(const models::ListPartsRequest& request,
                        const ListPartsAsyncCallback& callback,
                        const OperationOptions* options = nullptr);

    // Async helper
    template<typename OutcomeT, typename RequestT, typename CallbackT>
    std::future<OutcomeT> callAsync(
            void(OSSAsyncClient::*method)(const RequestT&, const CallbackT&, const OperationOptions*),
            const RequestT& request,
            const OperationOptions* options = nullptr) {
        auto promise = std::make_shared<std::promise<OutcomeT>>();
        (this->*method)(request, CallbackT([promise](OutcomeT result) {
            promise->set_value(std::move(result));
        }), options);
        return promise->get_future();
    }

  private:
    std::shared_ptr<internal::AsyncClientImpl> client_;
};

} // namespace oss2
} // namespace alibabacloud
