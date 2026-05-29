#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


/*
 * The container that stores the information about the bucket.
 */
struct ALIBABACLOUD_OSS_API BucketSummary final {
    // The region in which the bucket is located.
    std::string region;

    // The time when the bucket was created. Format: `yyyy-mm-ddThh:mm:ss.timezone`.
    std::string creationDate;

    // The public endpoint of the region in which the bucket resides.
    std::string extranetEndpoint;

    // The internal endpoint of the region in which the bucket you access from ECS instances resides. The bucket and ECS
    // instances are in the same region.
    std::string intranetEndpoint;

    // The data center in which the bucket is located.
    std::string location;

    // The name of the bucket.
    std::string name;

    // The storage class of the bucket. Valid values: Standard, IA, Archive, and ColdArchive.
    std::string storageClass;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    BucketSummary& setRegion(ValueT&& value) {
        region = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setCreationDate(ValueT&& value) {
        creationDate = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setExtranetEndpoint(ValueT&& value) {
        extranetEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setIntranetEndpoint(ValueT&& value) {
        intranetEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setLocation(ValueT&& value) {
        location = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setName(ValueT&& value) {
        name = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    BucketSummary& setStorageClass(ValueT&& value) {
        storageClass = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the information about multiple buckets.
 */
struct ALIBABACLOUD_OSS_API Buckets final {
    // The container that stores the information list of multiple buckets.
    std::vector<BucketSummary> buckets;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<BucketSummary>>
    Buckets& setBuckets(ValueT&& value) {
        buckets = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the result of listBuckets(GetService) request.
 */
struct ALIBABACLOUD_OSS_API ListAllMyBucketsResult final {
    std::optional<std::int64_t> maxKeys;
    std::optional<bool> isTruncated;
    std::string nextMarker;
    std::vector<BucketSummary> buckets;
    Owner owner;
    std::string prefix;
    std::string marker;
};


// The request for the ListBuckets operation.
class ALIBABACLOUD_OSS_API ListBucketsRequest final : public RequestModel {
  public:
    ListBucketsRequest() = default;

    // The ID of the resource group to which the bucket belongs.
    inline const std::string& getResourceGroupId() const {
        return getHeaderOrEmpty("x-oss-resource-group-id");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setResourceGroupId(ValueT&& value) {
        headers_.insert_or_assign("x-oss-resource-group-id", std::forward<ValueT>(value));
        return *this;
    }

    // The prefix that the names of returned buckets must contain. If this parameter is not specified, prefixes are not
    // used to filter returned buckets. By default, this parameter is left empty.
    inline const std::string& getPrefix() const {
        return getParameterOrEmpty("prefix");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setPrefix(ValueT&& value) {
        parameters_.insert_or_assign("prefix", std::forward<ValueT>(value));
        return *this;
    }

    // The name of the bucket from which the buckets start to return. The buckets whose names are alphabetically after
    // the value of marker are returned. If this parameter is not specified, all results are returned. By default, this
    // parameter is left empty.
    inline const std::string& getMarker() const {
        return getParameterOrEmpty("marker");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setMarker(ValueT&& value) {
        parameters_.insert_or_assign("marker", std::forward<ValueT>(value));
        return *this;
    }

    // The maximum number of buckets that can be returned. Valid values: 1 to 1000. Default value: 100
    inline std::int64_t getMaxKeys() const {
        return getParameterAsInt64Or("max-keys");
    }
    template <typename ValueT = std::int64_t>
    ListBucketsRequest& setMaxKeys(ValueT&& value) {
        parameters_.insert_or_assign("max-keys", std::to_string(std::forward<ValueT>(value)));
        return *this;
    }

    // A tag key of target buckets. The listing results will only include Buckets that have been tagged with this key.
    inline const std::string& getTagKey() const {
        return getParameterOrEmpty("tag-key");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setTagKey(ValueT&& value) {
        parameters_.insert_or_assign("tag-key", std::forward<ValueT>(value));
        return *this;
    }

    // A tag value for the target buckets. If this parameter is specified in the request, the tag-key must also be
    // specified. The listing results will only include Buckets that have been tagged with this key-value pair.
    inline const std::string& getTagValue() const {
        return getParameterOrEmpty("tag-value");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setTagValue(ValueT&& value) {
        parameters_.insert_or_assign("tag-value", std::forward<ValueT>(value));
        return *this;
    }

    // Tag list of target buckets. Only Buckets that match all the key-value pairs in the list will added into the
    // listing results. The tagging parameter cannot be used with the tag-key and tag-value parameters in a request.
    inline const std::string& getTagging() const {
        return getParameterOrEmpty("tagging");
    }
    template <typename ValueT = std::string>
    ListBucketsRequest& setTagging(ValueT&& value) {
        parameters_.insert_or_assign("tagging", std::forward<ValueT>(value));
        return *this;
    }


  private:
};


/// The result for the ListBuckets operation.
class ALIBABACLOUD_OSS_API ListBucketsResult final : public ResultModel {
  public:
    ListBucketsResult() = default;
    ListBucketsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}
    ListBucketsResult(int statusCode, HeaderCollection headers, ListAllMyBucketsResult body)
        : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // The maximum number of buckets that can be returned.
    // -1 means unkonwn
    std::int64_t getMaxKeys() const {
        return body_.maxKeys.value_or(-1LL);
    }

    // Indicates whether all results are returned. Valid values:- true: All results are not returned in the response. -
    // false: All results are returned in the response.
    bool getIsTruncated() const {
        return body_.isTruncated.value_or(false);
    }

    // The marker for the next listBuckets (GetService) request. You can use the value of this parameter as the value of
    // marker in the next listBuckets (GetService) request to retrieve the unreturned results.
    const std::string& getNextMarker() const {
        return body_.nextMarker;
    }

    // The container that stores the information list of multiple buckets.
    const std::vector<BucketSummary>& getBuckets() const {
        return body_.buckets;
    }

    // The container that stores the information about the bucket owner.
    const Owner& getOwner() const {
        return body_.owner;
    }

    // The prefix contained in the names of returned buckets.
    const std::string& getPrefix() const {
        return body_.prefix;
    }

    // The name of the bucket from which the buckets are returned.
    const std::string& getMarker() {
        return body_.marker;
    }

  private:
    ListAllMyBucketsResult body_;
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
