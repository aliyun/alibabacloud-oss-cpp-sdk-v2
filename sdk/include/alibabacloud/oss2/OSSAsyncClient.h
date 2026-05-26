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

    // Bucket Versioning

    /**
     * @brief Configures the versioning state for a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void putBucketVersioningAsync(const models::PutBucketVersioningRequest& request,
                                  const PutBucketVersioningAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    /**
     * @brief Queries the versioning state of a bucket.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void getBucketVersioningAsync(const models::GetBucketVersioningRequest& request,
                                  const GetBucketVersioningAsyncCallback& callback,
                                  const OperationOptions* options = nullptr);

    /**
     * @brief Lists the versions of all objects in a bucket, including delete markers.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void listObjectVersionsAsync(const models::ListObjectVersionsRequest& request,
                                  const ListObjectVersionsAsyncCallback& callback,
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

    // Object Select

    /**
     * @brief Executes a SQL query against a CSV or JSON object stored in OSS.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void selectObjectAsync(const models::SelectObjectRequest& request,
                           const SelectObjectAsyncCallback& callback,
                           const OperationOptions* options = nullptr);

    /**
     * @brief Creates or retrieves select metadata (row/column counts) for a CSV or JSON object.
     *
     * @param request The request parameter to send
     * @param callback The callback to receive the operation result
     * @param options Optional, operation options
     */
    void createSelectObjectMetaAsync(const models::CreateSelectObjectMetaRequest& request,
                                     const CreateSelectObjectMetaAsyncCallback& callback,
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

  public:
    // Extension

    /**
     * @brief Asynchronously uploads a local file as an object.
     */
    void putObjectFromFileAsync(const models::PutObjectRequest& request,
                                const std::string& filePath,
                                const PutObjectAsyncCallback& callback,
                                const OperationOptions* options = nullptr);

    /**
     * @brief Asynchronously downloads an object to a local file.
     *
     * Unlike the synchronous OSSClient::getObjectToFile which performs resumable retry
     * from the last written offset, this method is a convenience wrapper around
     * getObjectAsync that writes the response body to a file. Retries are handled
     * internally by the SDK at the request level (each retry downloads from the beginning).
     * Supports CRC-64 verification (controlled by EnableCRC64CheckDownload feature flag).
     */
    void getObjectToFileAsync(const models::GetObjectRequest& request,
                              const std::string& filePath,
                              const GetObjectAsyncCallback& callback,
                              const OperationOptions* options = nullptr);

    /**
     * @brief Asynchronously checks whether an object exists.
     */
    void isObjectExistAsync(const std::string& bucket, const std::string& key,
                            const BoolAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    /**
     * @brief Asynchronously checks whether a bucket exists.
     */
    void isBucketExistAsync(const std::string& bucket,
                            const BoolAsyncCallback& callback,
                            const OperationOptions* options = nullptr);

    template<typename RequestT>
    struct OperationTraits;

    /**
     * @brief Asynchronously invokes an operation and returns a std::future.
     *
     * The outcome type is automatically deduced from the request type via OperationTraits,
     * so no explicit template parameter or method pointer is needed.
     *
     * @code
     * auto future = client.asyncCall(
     *     models::GetObjectRequest().setBucket("my-bucket").setKey("my-key"));
     * auto outcome = future.get();
     * if (outcome.isSuccess()) {
     *     auto& result = outcome.getResult();
     *     // use result ...
     * }
     * @endcode
     *
     * @param request The request parameter to send
     * @param options Optional, operation options
     * @return std::future holding the operation outcome
     */
    template<typename RequestT>
    std::future<typename OperationTraits<RequestT>::OutcomeType>
    asyncCall(const RequestT& request, const OperationOptions* options = nullptr) {
        using Traits = OperationTraits<RequestT>;
        auto promise = std::make_shared<std::promise<typename Traits::OutcomeType>>();
        (this->*Traits::method)(request,
            typename Traits::CallbackType([promise](typename Traits::OutcomeType result) {
                promise->set_value(std::move(result));
            }), options);
        return promise->get_future();
    }

  private:
    std::shared_ptr<internal::AsyncClientImpl> client_;
};

// OperationTraits specializations

// Service
template<>
struct OSSAsyncClient::OperationTraits<models::ListBucketsRequest> {
    using OutcomeType = ListBucketsOutcome;
    using CallbackType = ListBucketsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listBucketsAsync;
};

// Region
template<>
struct OSSAsyncClient::OperationTraits<models::DescribeRegionsRequest> {
    using OutcomeType = DescribeRegionsOutcome;
    using CallbackType = DescribeRegionsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::describeRegionsAsync;
};

// Bucket Basic
template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketStatRequest> {
    using OutcomeType = GetBucketStatOutcome;
    using CallbackType = GetBucketStatAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketStatAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::PutBucketRequest> {
    using OutcomeType = PutBucketOutcome;
    using CallbackType = PutBucketAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putBucketAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::DeleteBucketRequest> {
    using OutcomeType = DeleteBucketOutcome;
    using CallbackType = DeleteBucketAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::deleteBucketAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::ListObjectsRequest> {
    using OutcomeType = ListObjectsOutcome;
    using CallbackType = ListObjectsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listObjectsAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::ListObjectsV2Request> {
    using OutcomeType = ListObjectsV2Outcome;
    using CallbackType = ListObjectsV2AsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listObjectsV2Async;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketInfoRequest> {
    using OutcomeType = GetBucketInfoOutcome;
    using CallbackType = GetBucketInfoAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketInfoAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketLocationRequest> {
    using OutcomeType = GetBucketLocationOutcome;
    using CallbackType = GetBucketLocationAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketLocationAsync;
};

// Bucket Acl
template<>
struct OSSAsyncClient::OperationTraits<models::PutBucketAclRequest> {
    using OutcomeType = PutBucketAclOutcome;
    using CallbackType = PutBucketAclAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putBucketAclAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketAclRequest> {
    using OutcomeType = GetBucketAclOutcome;
    using CallbackType = GetBucketAclAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketAclAsync;
};

// Bucket Referer
template<>
struct OSSAsyncClient::OperationTraits<models::PutBucketRefererRequest> {
    using OutcomeType = PutBucketRefererOutcome;
    using CallbackType = PutBucketRefererAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putBucketRefererAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketRefererRequest> {
    using OutcomeType = GetBucketRefererOutcome;
    using CallbackType = GetBucketRefererAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketRefererAsync;
};

// Bucket Versioning
template<>
struct OSSAsyncClient::OperationTraits<models::PutBucketVersioningRequest> {
    using OutcomeType = PutBucketVersioningOutcome;
    using CallbackType = PutBucketVersioningAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putBucketVersioningAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetBucketVersioningRequest> {
    using OutcomeType = GetBucketVersioningOutcome;
    using CallbackType = GetBucketVersioningAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getBucketVersioningAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::ListObjectVersionsRequest> {
    using OutcomeType = ListObjectVersionsOutcome;
    using CallbackType = ListObjectVersionsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listObjectVersionsAsync;
};

// Object Basic
template<>
struct OSSAsyncClient::OperationTraits<models::PutObjectRequest> {
    using OutcomeType = PutObjectOutcome;
    using CallbackType = PutObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::CopyObjectRequest> {
    using OutcomeType = CopyObjectOutcome;
    using CallbackType = CopyObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::copyObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetObjectRequest> {
    using OutcomeType = GetObjectOutcome;
    using CallbackType = GetObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::AppendObjectRequest> {
    using OutcomeType = AppendObjectOutcome;
    using CallbackType = AppendObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::appendObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::SealAppendObjectRequest> {
    using OutcomeType = SealAppendObjectOutcome;
    using CallbackType = SealAppendObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::sealAppendObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::DeleteObjectRequest> {
    using OutcomeType = DeleteObjectOutcome;
    using CallbackType = DeleteObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::deleteObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::DeleteMultipleObjectsRequest> {
    using OutcomeType = DeleteMultipleObjectsOutcome;
    using CallbackType = DeleteMultipleObjectsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::deleteMultipleObjectsAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::HeadObjectRequest> {
    using OutcomeType = HeadObjectOutcome;
    using CallbackType = HeadObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::headObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetObjectMetaRequest> {
    using OutcomeType = GetObjectMetaOutcome;
    using CallbackType = GetObjectMetaAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getObjectMetaAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::RestoreObjectRequest> {
    using OutcomeType = RestoreObjectOutcome;
    using CallbackType = RestoreObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::restoreObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::CleanRestoredObjectRequest> {
    using OutcomeType = CleanRestoredObjectOutcome;
    using CallbackType = CleanRestoredObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::cleanRestoredObjectAsync;
};

// Object Acl
template<>
struct OSSAsyncClient::OperationTraits<models::PutObjectAclRequest> {
    using OutcomeType = PutObjectAclOutcome;
    using CallbackType = PutObjectAclAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putObjectAclAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetObjectAclRequest> {
    using OutcomeType = GetObjectAclOutcome;
    using CallbackType = GetObjectAclAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getObjectAclAsync;
};

// Object Symlink
template<>
struct OSSAsyncClient::OperationTraits<models::PutSymlinkRequest> {
    using OutcomeType = PutSymlinkOutcome;
    using CallbackType = PutSymlinkAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putSymlinkAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetSymlinkRequest> {
    using OutcomeType = GetSymlinkOutcome;
    using CallbackType = GetSymlinkAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getSymlinkAsync;
};

// Object Tagging
template<>
struct OSSAsyncClient::OperationTraits<models::PutObjectTaggingRequest> {
    using OutcomeType = PutObjectTaggingOutcome;
    using CallbackType = PutObjectTaggingAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::putObjectTaggingAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::GetObjectTaggingRequest> {
    using OutcomeType = GetObjectTaggingOutcome;
    using CallbackType = GetObjectTaggingAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::getObjectTaggingAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::DeleteObjectTaggingRequest> {
    using OutcomeType = DeleteObjectTaggingOutcome;
    using CallbackType = DeleteObjectTaggingAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::deleteObjectTaggingAsync;
};

// Object Multipart
template<>
struct OSSAsyncClient::OperationTraits<models::InitiateMultipartUploadRequest> {
    using OutcomeType = InitiateMultipartUploadOutcome;
    using CallbackType = InitiateMultipartUploadAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::initiateMultipartUploadAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::UploadPartRequest> {
    using OutcomeType = UploadPartOutcome;
    using CallbackType = UploadPartAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::uploadPartAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::CompleteMultipartUploadRequest> {
    using OutcomeType = CompleteMultipartUploadOutcome;
    using CallbackType = CompleteMultipartUploadAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::completeMultipartUploadAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::UploadPartCopyRequest> {
    using OutcomeType = UploadPartCopyOutcome;
    using CallbackType = UploadPartCopyAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::uploadPartCopyAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::AbortMultipartUploadRequest> {
    using OutcomeType = AbortMultipartUploadOutcome;
    using CallbackType = AbortMultipartUploadAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::abortMultipartUploadAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::ListMultipartUploadsRequest> {
    using OutcomeType = ListMultipartUploadsOutcome;
    using CallbackType = ListMultipartUploadsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listMultipartUploadsAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::ListPartsRequest> {
    using OutcomeType = ListPartsOutcome;
    using CallbackType = ListPartsAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::listPartsAsync;
};

// Object Select
template<>
struct OSSAsyncClient::OperationTraits<models::SelectObjectRequest> {
    using OutcomeType = SelectObjectOutcome;
    using CallbackType = SelectObjectAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::selectObjectAsync;
};

template<>
struct OSSAsyncClient::OperationTraits<models::CreateSelectObjectMetaRequest> {
    using OutcomeType = CreateSelectObjectMetaOutcome;
    using CallbackType = CreateSelectObjectMetaAsyncCallback;
    static constexpr auto method = &OSSAsyncClient::createSelectObjectMetaAsync;
};

} // namespace oss2
} // namespace alibabacloud
