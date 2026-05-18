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
 * The container that stores the versioning state of the bucket.
 */
struct ALIBABACLOUD_OSS_API VersioningConfiguration final {
    // The versioning state of the bucket. Valid values: Enabled, Suspended.
    std::optional<std::string> status;

    template <typename ValueT = std::string>
    VersioningConfiguration& setStatus(ValueT&& value) {
        status = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the versions of objects, excluding delete markers.
 */
struct ALIBABACLOUD_OSS_API ObjectVersion final {
    // The name of the object.
    std::string key;

    // The version ID of the object.
    std::string versionId;

    // Indicates whether the version is the current version.
    std::optional<bool> isLatest;

    // The time when the object was last modified.
    std::string lastModified;

    // The ETag of the object.
    std::string eTag;

    // The size of the object. Unit: bytes.
    std::int64_t size{0};

    // The storage class of the object.
    std::string storageClass;

    // The container for the information about the bucket owner.
    std::optional<Owner> owner;

    // The restoration status of the object version.
    std::string restoreInfo;


    template <typename ValueT = std::string>
    ObjectVersion& setKey(ValueT&& value) { key = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    ObjectVersion& setVersionId(ValueT&& value) { versionId = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = bool>
    ObjectVersion& setIsLatest(ValueT&& value) { isLatest = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    ObjectVersion& setLastModified(ValueT&& value) { lastModified = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    ObjectVersion& setETag(ValueT&& value) { eTag = std::forward<ValueT>(value); return *this; }

    ObjectVersion& setSize(std::int64_t value) { size = value; return *this; }

    template <typename ValueT = std::string>
    ObjectVersion& setStorageClass(ValueT&& value) { storageClass = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = Owner>
    ObjectVersion& setOwner(ValueT&& value) { owner = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    ObjectVersion& setRestoreInfo(ValueT&& value) { restoreInfo = std::forward<ValueT>(value); return *this; }
};


/*
 * The container that stores delete markers.
 */
struct ALIBABACLOUD_OSS_API DeleteMarkerEntry final {
    // The name of the object.
    std::string key;

    // The version ID of the object.
    std::string versionId;

    // Indicates whether the version is the current version.
    std::optional<bool> isLatest;

    // The time when the object was last modified.
    std::string lastModified;

    // The container for the information about the bucket owner.
    std::optional<Owner> owner;


    template <typename ValueT = std::string>
    DeleteMarkerEntry& setKey(ValueT&& value) { key = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    DeleteMarkerEntry& setVersionId(ValueT&& value) { versionId = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = bool>
    DeleteMarkerEntry& setIsLatest(ValueT&& value) { isLatest = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = std::string>
    DeleteMarkerEntry& setLastModified(ValueT&& value) { lastModified = std::forward<ValueT>(value); return *this; }

    template <typename ValueT = Owner>
    DeleteMarkerEntry& setOwner(ValueT&& value) { owner = std::forward<ValueT>(value); return *this; }
};


/*
 * The container that stores the results of the ListObjectVersions (GetBucketVersions) request.
 */
struct ALIBABACLOUD_OSS_API ListVersionsResultXml final {
    std::string name;
    std::string prefix;
    std::string keyMarker;
    std::string versionIdMarker;
    std::string nextKeyMarker;
    std::string nextVersionIdMarker;
    std::optional<std::int32_t> maxKeys;
    std::string delimiter;
    std::optional<bool> isTruncated;
    std::string encodingType;
    std::vector<ObjectVersion> versions;
    std::vector<DeleteMarkerEntry> deleteMarkers;
    std::vector<CommonPrefix> commonPrefixes;
};


// The request for the PutBucketVersioning operation.
class ALIBABACLOUD_OSS_API PutBucketVersioningRequest final : public RequestModel {
  public:
    PutBucketVersioningRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutBucketVersioningRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The request body schema.
    inline const VersioningConfiguration& getVersioningConfiguration() const {
        return body_.at(0);
    }

    inline bool hasVersioningConfiguration() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = VersioningConfiguration>
    PutBucketVersioningRequest& setVersioningConfiguration(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::map<int, VersioningConfiguration> body_;
};

/// The result for the PutBucketVersioning operation.
class ALIBABACLOUD_OSS_API PutBucketVersioningResult final : public ResultModel {
  public:
    PutBucketVersioningResult() = default;
    PutBucketVersioningResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the GetBucketVersioning operation.
class ALIBABACLOUD_OSS_API GetBucketVersioningRequest final : public RequestModel {
  public:
    GetBucketVersioningRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketVersioningRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketVersioning operation.
class ALIBABACLOUD_OSS_API GetBucketVersioningResult final : public ResultModel {
  public:
    GetBucketVersioningResult() = default;
    GetBucketVersioningResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the versioning state of the bucket.
    inline const VersioningConfiguration& getVersioningConfiguration() {
        return body_[0];
    }

    inline bool hasVersioningConfiguration() const {
        return bodyIsSet_;
    }

    template <typename ValueT = VersioningConfiguration>
    GetBucketVersioningResult& setVersioningConfiguration(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, VersioningConfiguration> body_;
    bool bodyIsSet_{};
};

// The request for the ListObjectVersions operation.
class ALIBABACLOUD_OSS_API ListObjectVersionsRequest final : public RequestModel {
  public:
    ListObjectVersionsRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The character that is used to group objects by name.
    inline const std::string& getDelimiter() const {
        return getParameterOrEmpty("delimiter");
    }
    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setDelimiter(ValueT&& value) {
        parameters_.insert_or_assign("delimiter", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the object after which the list operation begins.
    inline const std::string& getKeyMarker() const {
        return getParameterOrEmpty("key-marker");
    }
    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setKeyMarker(ValueT&& value) {
        parameters_.insert_or_assign("key-marker", std::forward<ValueT>(value));
        return *this;
    }

    // The version ID of the object specified in key-marker after which the list operation begins.
    inline const std::string& getVersionIdMarker() const {
        return getParameterOrEmpty("version-id-marker");
    }
    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setVersionIdMarker(ValueT&& value) {
        parameters_.insert_or_assign("version-id-marker", std::forward<ValueT>(value));
        return *this;
    }

    // The maximum number of objects to be returned.
    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    template <typename ValueT = std::int64_t>
    ListObjectVersionsRequest& setMaxKeys(ValueT&& value) {
        parameters_.insert_or_assign("max-keys", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // The prefix that the names of returned objects must contain.
    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    // The encoding type of the content in the response.
    inline const std::string& getEncodingType() const {
        return getParameterOrEmpty("encoding-type");
    }
    template <typename ValueT = std::string>
    ListObjectVersionsRequest& setEncodingType(ValueT&& value) {
        parameters_.insert_or_assign("encoding-type", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
};


/// The result for the ListObjectVersions operation.
class ALIBABACLOUD_OSS_API ListObjectVersionsResult final : public ResultModel {
  public:
    ListObjectVersionsResult() = default;
    ListObjectVersionsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    ListObjectVersionsResult(int statusCode, HeaderCollection headers, ListVersionsResultXml body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}


    // The bucket name.
    inline const std::string& getName() const {
        return body_.name;
    }

    // The prefix contained in the names of the returned objects.
    inline const std::string& getPrefix() const {
        return body_.prefix;
    }

    // Indicates the object from which the list operation starts.
    inline const std::string& getKeyMarker() const {
        return body_.keyMarker;
    }

    // The version from which the list operation starts.
    inline const std::string& getVersionIdMarker() const {
        return body_.versionIdMarker;
    }

    // If not all results are returned, the NextKeyMarker for the next request.
    inline const std::string& getNextKeyMarker() const {
        return body_.nextKeyMarker;
    }

    // If not all results are returned, the NextVersionIdMarker for the next request.
    inline const std::string& getNextVersionIdMarker() const {
        return body_.nextVersionIdMarker;
    }

    // The maximum number of objects that can be returned in the response.
    inline std::int32_t getMaxKeys() const {
        return body_.maxKeys.value_or(-1);
    }

    // The delimiter used to group objects by name.
    inline const std::string& getDelimiter() const {
        return body_.delimiter;
    }

    // Indicates whether the returned results are truncated.
    inline bool getIsTruncated() const {
        return body_.isTruncated.value_or(false);
    }

    // The encoding type of the content in the response.
    inline const std::string& getEncodingType() const {
        return body_.encodingType;
    }

    // The container that stores the versions of objects except for delete markers.
    inline const std::vector<ObjectVersion>& getVersions() const {
        return body_.versions;
    }

    // The container that stores delete markers.
    inline const std::vector<DeleteMarkerEntry>& getDeleteMarkers() const {
        return body_.deleteMarkers;
    }

    // Objects whose names contain the same string that ranges from the prefix to the next occurrence of the delimiter.
    inline const std::vector<CommonPrefix>& getCommonPrefixes() const {
        return body_.commonPrefixes;
    }


  private:
    ListVersionsResultXml body_;
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
