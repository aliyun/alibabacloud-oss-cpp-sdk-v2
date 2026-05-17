#pragma once

#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/OSSFwd.h"
#include "alibabacloud/oss2/Error.h"

#include <future>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>


namespace alibabacloud {
namespace oss2 {

// forward declare
struct ClientConfiguration;
class Executor;

namespace internal {
class ClientImpl;
}

class ALIBABACLOUD_OSS_API OSSClient final {
  public:
    explicit OSSClient(const struct ClientConfiguration& config);

    explicit OSSClient(const struct ClientConfiguration& config, ClientOptionsFns& fns);

    ~OSSClient() = default;

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

    template<typename RequestT>
    struct OperationTraits;

    /**
     * @brief Asynchronously invokes an operation on the executor and returns a std::future.
     *
     * The outcome type is automatically deduced from the request type via OperationTraits,
     * so no explicit template parameter or method pointer is needed.
     * Requires an Executor to be configured in ClientConfiguration; otherwise returns
     * a future containing a NoExecutor error.
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
    std::future<typename OperationTraits<std::decay_t<RequestT>>::OutcomeType>
    asyncCall(RequestT&& request, const OperationOptions* options = nullptr) {
        using Traits = OperationTraits<std::decay_t<RequestT>>;
        using OutcomeT = typename Traits::OutcomeType;
        if (!executor_) {
            std::promise<OutcomeT> p;
            p.set_value(OutcomeT(makeUnexpected(OperationError(ClientErrorCode::ArgumentInvalid,
                {{"Code", "NoExecutor"}, {"Message", "No executor configured for async operations"}}))));
            return p.get_future();
        }
        auto opts = options ? std::make_optional(*options) : std::nullopt;
        auto task = std::make_shared<std::packaged_task<OutcomeT()>>(
            [this, request = std::forward<RequestT>(request), opts = std::move(opts)]() {
                return (this->*Traits::method)(request, opts ? &*opts : nullptr);
            });
        executeTask([task]() { (*task)(); });
        return task->get_future();
    }

    /**
     * @brief Asynchronously invokes an operation on the executor and delivers the result via callback.
     *
     * The outcome type is automatically deduced from the request type via OperationTraits.
     * The handler signature is: void(const OSSClient*, const RequestT&, const OutcomeT&).
     * Requires an Executor to be configured in ClientConfiguration; otherwise the handler
     * is called immediately with a NoExecutor error.
     *
     * @code
     * client.asyncCallback(
     *     models::GetObjectRequest().setBucket("my-bucket").setKey("my-key"),
     *     [](const OSSClient*, const models::GetObjectRequest&, const GetObjectOutcome& outcome) {
     *         if (outcome.isSuccess()) {
     *             // use outcome.getResult() ...
     *         }
     *     });
     * @endcode
     *
     * @param request The request parameter to send
     * @param handler The callback to receive the operation result
     * @param options Optional, operation options
     */
    template<typename RequestT, typename HandlerT>
    void asyncCallback(RequestT&& request, HandlerT&& handler,
                       const OperationOptions* options = nullptr) {
        using Traits = OperationTraits<std::decay_t<RequestT>>;
        using OutcomeT = typename Traits::OutcomeType;
        if (!executor_) {
            handler(this, request, OutcomeT(makeUnexpected(OperationError(ClientErrorCode::ArgumentInvalid,
                {{"Code", "NoExecutor"}, {"Message", "No executor configured for async operations"}}))));
            return;
        }
        auto opts = options ? std::make_optional(*options) : std::nullopt;
        executeTask([this, req = std::forward<RequestT>(request),
                     h = std::forward<HandlerT>(handler),
                     opts = std::move(opts)]() {
            h(this, req, (this->*Traits::method)(req, opts ? &*opts : nullptr));
        });
    }

  private:
    void executeTask(std::function<void()> task);
    std::shared_ptr<internal::ClientImpl> client_;
    std::shared_ptr<Executor> executor_;
};

// OperationTraits specializations

// Service
template<> struct OSSClient::OperationTraits<models::ListBucketsRequest> {
    using OutcomeType = ListBucketsOutcome;
    static constexpr auto method = &OSSClient::listBuckets;
};

// Region
template<> struct OSSClient::OperationTraits<models::DescribeRegionsRequest> {
    using OutcomeType = DescribeRegionsOutcome;
    static constexpr auto method = &OSSClient::describeRegions;
};

// Bucket Basic
template<> struct OSSClient::OperationTraits<models::GetBucketStatRequest> {
    using OutcomeType = GetBucketStatOutcome;
    static constexpr auto method = &OSSClient::getBucketStat;
};

template<> struct OSSClient::OperationTraits<models::PutBucketRequest> {
    using OutcomeType = PutBucketOutcome;
    static constexpr auto method = &OSSClient::putBucket;
};

template<> struct OSSClient::OperationTraits<models::DeleteBucketRequest> {
    using OutcomeType = DeleteBucketOutcome;
    static constexpr auto method = &OSSClient::deleteBucket;
};

template<> struct OSSClient::OperationTraits<models::DeleteMultipleObjectsRequest> {
    using OutcomeType = DeleteMultipleObjectsOutcome;
    static constexpr auto method = &OSSClient::deleteMultipleObjects;
};

template<> struct OSSClient::OperationTraits<models::ListObjectsRequest> {
    using OutcomeType = ListObjectsOutcome;
    static constexpr auto method = &OSSClient::listObjects;
};

template<> struct OSSClient::OperationTraits<models::ListObjectsV2Request> {
    using OutcomeType = ListObjectsV2Outcome;
    static constexpr auto method = &OSSClient::listObjectsV2;
};

template<> struct OSSClient::OperationTraits<models::GetBucketInfoRequest> {
    using OutcomeType = GetBucketInfoOutcome;
    static constexpr auto method = &OSSClient::getBucketInfo;
};

template<> struct OSSClient::OperationTraits<models::GetBucketLocationRequest> {
    using OutcomeType = GetBucketLocationOutcome;
    static constexpr auto method = &OSSClient::getBucketLocation;
};

// Bucket Acl
template<> struct OSSClient::OperationTraits<models::PutBucketAclRequest> {
    using OutcomeType = PutBucketAclOutcome;
    static constexpr auto method = &OSSClient::putBucketAcl;
};

template<> struct OSSClient::OperationTraits<models::GetBucketAclRequest> {
    using OutcomeType = GetBucketAclOutcome;
    static constexpr auto method = &OSSClient::getBucketAcl;
};

// Bucket Referer
template<> struct OSSClient::OperationTraits<models::PutBucketRefererRequest> {
    using OutcomeType = PutBucketRefererOutcome;
    static constexpr auto method = &OSSClient::putBucketReferer;
};

template<> struct OSSClient::OperationTraits<models::GetBucketRefererRequest> {
    using OutcomeType = GetBucketRefererOutcome;
    static constexpr auto method = &OSSClient::getBucketReferer;
};

// Object Basic
template<> struct OSSClient::OperationTraits<models::PutObjectRequest> {
    using OutcomeType = PutObjectOutcome;
    static constexpr auto method = &OSSClient::putObject;
};

template<> struct OSSClient::OperationTraits<models::CopyObjectRequest> {
    using OutcomeType = CopyObjectOutcome;
    static constexpr auto method = &OSSClient::copyObject;
};

template<> struct OSSClient::OperationTraits<models::GetObjectRequest> {
    using OutcomeType = GetObjectOutcome;
    static constexpr auto method = &OSSClient::getObject;
};

template<> struct OSSClient::OperationTraits<models::AppendObjectRequest> {
    using OutcomeType = AppendObjectOutcome;
    static constexpr auto method = &OSSClient::appendObject;
};

template<> struct OSSClient::OperationTraits<models::SealAppendObjectRequest> {
    using OutcomeType = SealAppendObjectOutcome;
    static constexpr auto method = &OSSClient::sealAppendObject;
};

template<> struct OSSClient::OperationTraits<models::DeleteObjectRequest> {
    using OutcomeType = DeleteObjectOutcome;
    static constexpr auto method = &OSSClient::deleteObject;
};

template<> struct OSSClient::OperationTraits<models::HeadObjectRequest> {
    using OutcomeType = HeadObjectOutcome;
    static constexpr auto method = &OSSClient::headObject;
};

template<> struct OSSClient::OperationTraits<models::GetObjectMetaRequest> {
    using OutcomeType = GetObjectMetaOutcome;
    static constexpr auto method = &OSSClient::getObjectMeta;
};

template<> struct OSSClient::OperationTraits<models::RestoreObjectRequest> {
    using OutcomeType = RestoreObjectOutcome;
    static constexpr auto method = &OSSClient::restoreObject;
};

template<> struct OSSClient::OperationTraits<models::CleanRestoredObjectRequest> {
    using OutcomeType = CleanRestoredObjectOutcome;
    static constexpr auto method = &OSSClient::cleanRestoredObject;
};

// Object Acl
template<> struct OSSClient::OperationTraits<models::PutObjectAclRequest> {
    using OutcomeType = PutObjectAclOutcome;
    static constexpr auto method = &OSSClient::putObjectAcl;
};

template<> struct OSSClient::OperationTraits<models::GetObjectAclRequest> {
    using OutcomeType = GetObjectAclOutcome;
    static constexpr auto method = &OSSClient::getObjectAcl;
};

// Object Symlink
template<> struct OSSClient::OperationTraits<models::PutSymlinkRequest> {
    using OutcomeType = PutSymlinkOutcome;
    static constexpr auto method = &OSSClient::putSymlink;
};

template<> struct OSSClient::OperationTraits<models::GetSymlinkRequest> {
    using OutcomeType = GetSymlinkOutcome;
    static constexpr auto method = &OSSClient::getSymlink;
};

// Object Tagging
template<> struct OSSClient::OperationTraits<models::PutObjectTaggingRequest> {
    using OutcomeType = PutObjectTaggingOutcome;
    static constexpr auto method = &OSSClient::putObjectTagging;
};

template<> struct OSSClient::OperationTraits<models::GetObjectTaggingRequest> {
    using OutcomeType = GetObjectTaggingOutcome;
    static constexpr auto method = &OSSClient::getObjectTagging;
};

template<> struct OSSClient::OperationTraits<models::DeleteObjectTaggingRequest> {
    using OutcomeType = DeleteObjectTaggingOutcome;
    static constexpr auto method = &OSSClient::deleteObjectTagging;
};

// Object Multipart
template<> struct OSSClient::OperationTraits<models::InitiateMultipartUploadRequest> {
    using OutcomeType = InitiateMultipartUploadOutcome;
    static constexpr auto method = &OSSClient::initiateMultipartUpload;
};

template<> struct OSSClient::OperationTraits<models::UploadPartRequest> {
    using OutcomeType = UploadPartOutcome;
    static constexpr auto method = &OSSClient::uploadPart;
};

template<> struct OSSClient::OperationTraits<models::CompleteMultipartUploadRequest> {
    using OutcomeType = CompleteMultipartUploadOutcome;
    static constexpr auto method = &OSSClient::completeMultipartUpload;
};

template<> struct OSSClient::OperationTraits<models::UploadPartCopyRequest> {
    using OutcomeType = UploadPartCopyOutcome;
    static constexpr auto method = &OSSClient::uploadPartCopy;
};

template<> struct OSSClient::OperationTraits<models::AbortMultipartUploadRequest> {
    using OutcomeType = AbortMultipartUploadOutcome;
    static constexpr auto method = &OSSClient::abortMultipartUpload;
};

template<> struct OSSClient::OperationTraits<models::ListMultipartUploadsRequest> {
    using OutcomeType = ListMultipartUploadsOutcome;
    static constexpr auto method = &OSSClient::listMultipartUploads;
};

template<> struct OSSClient::OperationTraits<models::ListPartsRequest> {
    using OutcomeType = ListPartsOutcome;
    static constexpr auto method = &OSSClient::listParts;
};

} // namespace oss2
} // namespace alibabacloud