#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/BucketBasic.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace agentic {
namespace models {

using alibabacloud::oss2::models::Owner;
using alibabacloud::oss2::models::ServerSideEncryptionRule;

/*
 * The configurations used to create an agentic bucket.
 */
struct ALIBABACLOUD_OSS_API CreateAgenticBucketConfiguration final {
    // The storage class of the agentic bucket.
    std::optional<std::string> storageClass;

    // The redundancy type of the agentic bucket.
    std::optional<std::string> dataRedundancyType;

    template <typename ValueT = std::string>
    CreateAgenticBucketConfiguration& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    CreateAgenticBucketConfiguration& setDataRedundancyType(ValueT&& value) {
        dataRedundancyType = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The status of an agentic bucket.
 */
struct ALIBABACLOUD_OSS_API AgenticBucketStatus final {
    // The status of the agentic bucket.
    std::string status;

    template <typename ValueT = std::string>
    AgenticBucketStatus& setStatus(ValueT&& value) {
        status = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The container that stores the agentic bucket information.
 */
struct ALIBABACLOUD_OSS_API AgenticBucketInfo final {
    // The name of the agentic bucket.
    std::optional<std::string> name;

    // The owner of the agentic bucket.
    std::optional<std::string> owner;

    // The region in which the agentic bucket is located.
    std::optional<std::string> region;

    // The storage class of the agentic bucket.
    std::optional<std::string> storageClass;

    // The redundancy type of the agentic bucket.
    std::optional<std::string> dataRedundancyType;

    // The status of the agentic bucket.
    std::optional<std::string> status;

    // The resource type of the agentic bucket.
    std::optional<std::string> bucketResourceType;

    // The time when the agentic bucket is created.
    std::optional<std::string> createTime;

    // The ACL of the agentic bucket.
    std::optional<std::string> acl;

    // Whether the agentic bucket has been configured to block public access.
    std::optional<std::string> publicAccessBlock;

    // The server-side encryption configurations of the agentic bucket.
    std::optional<ServerSideEncryptionRule> serverSideEncryptionRule;

    // The versioning status of the agentic bucket.
    std::optional<std::string> versioning;

    // The bucket policy of the agentic bucket.
    std::optional<std::string> bucketPolicy;

    template <typename ValueT = std::string>
    AgenticBucketInfo& setName(ValueT&& value) {
        name = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setOwner(ValueT&& value) {
        owner = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setRegion(ValueT&& value) {
        region = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setDataRedundancyType(ValueT&& value) {
        dataRedundancyType = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setStatus(ValueT&& value) {
        status = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setBucketResourceType(ValueT&& value) {
        bucketResourceType = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setCreateTime(ValueT&& value) {
        createTime = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setAcl(ValueT&& value) {
        acl = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setPublicAccessBlock(ValueT&& value) {
        publicAccessBlock = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = ServerSideEncryptionRule>
    AgenticBucketInfo& setServerSideEncryptionRule(ValueT&& value) {
        serverSideEncryptionRule = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setVersioning(ValueT&& value) {
        versioning = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketInfo& setBucketPolicy(ValueT&& value) {
        bucketPolicy = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The summary of an agentic bucket returned by the ListAgenticBuckets operation.
 */
struct ALIBABACLOUD_OSS_API AgenticBucketSummary final {
    // The name of the agentic bucket.
    std::optional<std::string> name;

    // The storage class of the agentic bucket.
    std::optional<std::string> storageClass;

    // The redundancy type of the agentic bucket.
    std::optional<std::string> dataRedundancyType;

    // The time when the agentic bucket is created.
    std::optional<std::string> createTime;

    template <typename ValueT = std::string>
    AgenticBucketSummary& setName(ValueT&& value) {
        name = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketSummary& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketSummary& setDataRedundancyType(ValueT&& value) {
        dataRedundancyType = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    AgenticBucketSummary& setCreateTime(ValueT&& value) {
        createTime = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The summary of a bucket space returned by the ListBucketSpaces operation.
 */
struct ALIBABACLOUD_OSS_API BucketSpaceSummary final {
    // The name of the bucket space.
    std::optional<std::string> name;

    // The location of the bucket space.
    std::optional<std::string> location;

    // The time when the bucket space is created.
    std::optional<std::string> creationDate;

    // The storage class of the bucket space.
    std::optional<std::string> storageClass;

    template <typename ValueT = std::string>
    BucketSpaceSummary& setName(ValueT&& value) {
        name = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSpaceSummary& setLocation(ValueT&& value) {
        location = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSpaceSummary& setCreationDate(ValueT&& value) {
        creationDate = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSpaceSummary& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }
};

// The request for the CreateAgenticBucket operation.
class ALIBABACLOUD_OSS_API CreateAgenticBucketRequest final : public RequestModel {
  public:
    CreateAgenticBucketRequest() = default;

    inline const std::string& getBucket() const {
        return bucket_;
    }
    template <typename ValueT = std::string>
    CreateAgenticBucketRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const CreateAgenticBucketConfiguration& getCreateAgenticBucketConfiguration() const {
        return body_.at(0);
    }
    inline bool hasCreateAgenticBucketConfiguration() const {
        return body_.find(0) != body_.end();
    }
    template <typename ValueT = CreateAgenticBucketConfiguration>
    CreateAgenticBucketRequest& setCreateAgenticBucketConfiguration(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

  private:
    std::string bucket_;
    std::map<int, CreateAgenticBucketConfiguration> body_;
};

// The result for the CreateAgenticBucket operation.
class ALIBABACLOUD_OSS_API CreateAgenticBucketResult final : public ResultModel {
  public:
    CreateAgenticBucketResult() = default;
    CreateAgenticBucketResult(int statusCode, HeaderCollection headers)
        : ResultModel(statusCode, std::move(headers)) {}
};

// The request for the DeleteAgenticBucket operation.
class ALIBABACLOUD_OSS_API DeleteAgenticBucketRequest final : public RequestModel {
  public:
    DeleteAgenticBucketRequest() = default;

    inline const std::string& getBucket() const {
        return bucket_;
    }
    template <typename ValueT = std::string>
    DeleteAgenticBucketRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string bucket_;
};

// The result for the DeleteAgenticBucket operation.
class ALIBABACLOUD_OSS_API DeleteAgenticBucketResult final : public ResultModel {
  public:
    DeleteAgenticBucketResult() = default;
    DeleteAgenticBucketResult(int statusCode, HeaderCollection headers)
        : ResultModel(statusCode, std::move(headers)) {}
};

// The request for the GetAgenticBucket operation.
class ALIBABACLOUD_OSS_API GetAgenticBucketRequest final : public RequestModel {
  public:
    GetAgenticBucketRequest() = default;

    inline const std::string& getBucket() const {
        return bucket_;
    }
    template <typename ValueT = std::string>
    GetAgenticBucketRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string bucket_;
};

// The result for the GetAgenticBucket operation.
class ALIBABACLOUD_OSS_API GetAgenticBucketResult final : public ResultModel {
  public:
    GetAgenticBucketResult() = default;
    GetAgenticBucketResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}

    inline const AgenticBucketInfo& getAgenticBucketInfo() {
        return body_[0];
    }
    inline bool hasAgenticBucketInfo() const {
        return bodyIsSet_;
    }
    template <typename ValueT = AgenticBucketInfo>
    GetAgenticBucketResult& setAgenticBucketInfo(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

  private:
    std::map<int, AgenticBucketInfo> body_;
    bool bodyIsSet_{};
};

// The request for the ListAgenticBuckets operation.
class ALIBABACLOUD_OSS_API ListAgenticBucketsRequest final : public RequestModel {
  public:
    ListAgenticBucketsRequest() = default;

    inline const std::string& getContinuationToken() const {
        return getParameterOrEmpty("continuation-token");
    }
    template <typename ValueT = std::string>
    ListAgenticBucketsRequest& setContinuationToken(ValueT&& value) {
        parameters_.insert_or_assign("continuation-token", std::forward<ValueT>(value));
        return *this;
    }

    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    inline ListAgenticBucketsRequest& setMaxKeys(std::int64_t value) {
        parameters_.insert_or_assign("max-keys", std::to_string(value));
        return *this;
    }
};

// The result for the ListAgenticBuckets operation.
class ALIBABACLOUD_OSS_API ListAgenticBucketsResult final : public ResultModel {
  public:
    ListAgenticBucketsResult() = default;
    ListAgenticBucketsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}

    inline const std::optional<std::string>& getRegion() const {
        return region_;
    }
    template <typename ValueT = std::string>
    ListAgenticBucketsResult& setRegion(ValueT&& value) {
        region_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<std::string>& getOwner() const {
        return owner_;
    }
    template <typename ValueT = std::string>
    ListAgenticBucketsResult& setOwner(ValueT&& value) {
        owner_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<std::string>& getContinuationToken() const {
        return continuationToken_;
    }
    template <typename ValueT = std::string>
    ListAgenticBucketsResult& setContinuationToken(ValueT&& value) {
        continuationToken_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<std::string>& getNextContinuationToken() const {
        return nextContinuationToken_;
    }
    template <typename ValueT = std::string>
    ListAgenticBucketsResult& setNextContinuationToken(ValueT&& value) {
        nextContinuationToken_ = std::forward<ValueT>(value);
        return *this;
    }

    inline bool getIsTruncated() const {
        return isTruncated_;
    }
    inline ListAgenticBucketsResult& setIsTruncated(bool value) {
        isTruncated_ = value;
        return *this;
    }

    inline const std::vector<AgenticBucketSummary>& getAgenticBuckets() const {
        return agenticBuckets_;
    }
    template <typename ValueT = std::vector<AgenticBucketSummary>>
    ListAgenticBucketsResult& setAgenticBuckets(ValueT&& value) {
        agenticBuckets_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::optional<std::string> region_;
    std::optional<std::string> owner_;
    std::optional<std::string> continuationToken_;
    std::optional<std::string> nextContinuationToken_;
    bool isTruncated_{false};
    std::vector<AgenticBucketSummary> agenticBuckets_;
};

// The request for the PutAgenticBucketStatus operation.
class ALIBABACLOUD_OSS_API PutAgenticBucketStatusRequest final : public RequestModel {
  public:
    PutAgenticBucketStatusRequest() = default;

    inline const std::string& getBucket() const {
        return bucket_;
    }
    template <typename ValueT = std::string>
    PutAgenticBucketStatusRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const AgenticBucketStatus& getAgenticBucketStatus() const {
        return body_.at(0);
    }
    inline bool hasAgenticBucketStatus() const {
        return body_.find(0) != body_.end();
    }
    template <typename ValueT = AgenticBucketStatus>
    PutAgenticBucketStatusRequest& setAgenticBucketStatus(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

  private:
    std::string bucket_;
    std::map<int, AgenticBucketStatus> body_;
};

// The result for the PutAgenticBucketStatus operation.
class ALIBABACLOUD_OSS_API PutAgenticBucketStatusResult final : public ResultModel {
  public:
    PutAgenticBucketStatusResult() = default;
    PutAgenticBucketStatusResult(int statusCode, HeaderCollection headers)
        : ResultModel(statusCode, std::move(headers)) {}
};

// The request for the ListBucketSpaces operation.
class ALIBABACLOUD_OSS_API ListBucketSpacesRequest final : public RequestModel {
  public:
    ListBucketSpacesRequest() = default;

    inline const std::string& getBucket() const {
        return bucket_;
    }
    template <typename ValueT = std::string>
    ListBucketSpacesRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListBucketSpacesRequest& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    inline const std::string& getContinuationToken() const {
        return getParameterOrEmpty("continuation-token");
    }
    template <typename ValueT = std::string>
    ListBucketSpacesRequest& setContinuationToken(ValueT&& value) {
        parameters_.insert_or_assign("continuation-token", std::forward<ValueT>(value));
        return *this;
    }

    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    inline ListBucketSpacesRequest& setMaxKeys(std::int64_t value) {
        parameters_.insert_or_assign("max-keys", std::to_string(value));
        return *this;
    }

  private:
    std::string bucket_;
};

// The result for the ListBucketSpaces operation.
class ALIBABACLOUD_OSS_API ListBucketSpacesResult final : public ResultModel {
  public:
    ListBucketSpacesResult() = default;
    ListBucketSpacesResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}

    inline const std::optional<Owner>& getOwner() const {
        return owner_;
    }
    template <typename ValueT = Owner>
    ListBucketSpacesResult& setOwner(ValueT&& value) {
        owner_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<std::string>& getPrefix() const {
        return prefix_;
    }
    template <typename ValueT = std::string>
    ListBucketSpacesResult& setPrefix(ValueT&& value) {
        prefix_ = std::forward<ValueT>(value);
        return *this;
    }

    inline std::int32_t getMaxKeys() const {
        return maxKeys_;
    }
    inline ListBucketSpacesResult& setMaxKeys(std::int32_t value) {
        maxKeys_ = value;
        return *this;
    }

    inline const std::optional<std::string>& getContinuationToken() const {
        return continuationToken_;
    }
    template <typename ValueT = std::string>
    ListBucketSpacesResult& setContinuationToken(ValueT&& value) {
        continuationToken_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::optional<std::string>& getNextContinuationToken() const {
        return nextContinuationToken_;
    }
    template <typename ValueT = std::string>
    ListBucketSpacesResult& setNextContinuationToken(ValueT&& value) {
        nextContinuationToken_ = std::forward<ValueT>(value);
        return *this;
    }

    inline bool getIsTruncated() const {
        return isTruncated_;
    }
    inline ListBucketSpacesResult& setIsTruncated(bool value) {
        isTruncated_ = value;
        return *this;
    }

    inline const std::vector<BucketSpaceSummary>& getBucketSpaces() const {
        return bucketSpaces_;
    }
    template <typename ValueT = std::vector<BucketSpaceSummary>>
    ListBucketSpacesResult& setBucketSpaces(ValueT&& value) {
        bucketSpaces_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::optional<Owner> owner_;
    std::optional<std::string> prefix_;
    std::int32_t maxKeys_{0};
    std::optional<std::string> continuationToken_;
    std::optional<std::string> nextContinuationToken_;
    bool isTruncated_{false};
    std::vector<BucketSpaceSummary> bucketSpaces_;
};

} // namespace models
} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
