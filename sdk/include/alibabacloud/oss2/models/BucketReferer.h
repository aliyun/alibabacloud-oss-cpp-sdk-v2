#pragma once

#include "alibabacloud/oss2/Types.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


/*
 * The container that stores the Referer blacklist.
 */
struct ALIBABACLOUD_OSS_API RefererBlacklist final {
    // The addresses in the Referer blacklist.
    std::vector<std::string> referers;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<std::string>>
    RefererBlacklist& setReferers(ValueT&& value) {
        referers = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the Referer whitelist.  ****The PutBucketReferer operation overwrites the existing Referer
 * whitelist with the Referer whitelist specified in RefererList. If RefererList is not specified in the request, which
 * specifies that no Referer elements are included, the operation clears the existing Referer whitelist.
 */
struct ALIBABACLOUD_OSS_API RefererList final {
    // The addresses in the Referer whitelist.
    std::vector<std::string> referers;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<std::string>>
    RefererList& setReferers(ValueT&& value) {
        referers = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the hotlink protection configurations.
 */
struct ALIBABACLOUD_OSS_API RefererConfiguration final {
    // Specifies whether to allow a request whose Referer field is empty. Valid values:*   true (default)*   false
    std::optional<bool> allowEmptyReferer;

    // Specifies whether to truncate the query string in the URL when the Referer is matched. Valid values:*   true
    // (default)*   false
    std::optional<bool> allowTruncateQueryString;

    // Specifies whether to truncate the path and parts that follow the path in the URL when the Referer is matched.
    // Valid values:*   true*   false
    std::optional<bool> truncatePath;

    // The container that stores the Referer whitelist.  ****The PutBucketReferer operation overwrites the existing
    // Referer whitelist with the Referer whitelist specified in RefererList. If RefererList is not specified in the
    // request, which specifies that no Referer elements are included, the operation clears the existing Referer
    // whitelist.
    std::optional<RefererList> refererList;

    // The container that stores the Referer blacklist.
    std::optional<RefererBlacklist> refererBlacklist;


    // Provide setter interfaces via template
    template <typename ValueT = bool>
    RefererConfiguration& setAllowEmptyReferer(ValueT&& value) {
        allowEmptyReferer = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = bool>
    RefererConfiguration& setAllowTruncateQueryString(ValueT&& value) {
        allowTruncateQueryString = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = bool>
    RefererConfiguration& setTruncatePath(ValueT&& value) {
        truncatePath = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = RefererList>
    RefererConfiguration& setRefererList(ValueT&& value) {
        refererList = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = RefererBlacklist>
    RefererConfiguration& setRefererBlacklist(ValueT&& value) {
        refererBlacklist = std::forward<ValueT>(value);
        return *this;
    }
};


// The request for the PutBucketReferer operation.
class ALIBABACLOUD_OSS_API PutBucketRefererRequest final : public RequestModel {
  public:
    PutBucketRefererRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutBucketRefererRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The request body schema.
    inline const RefererConfiguration& getRefererConfiguration() const {
        return body_.at(0);
    }

    inline bool hasRefererConfiguration() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = RefererConfiguration>
    PutBucketRefererRequest& setRefererConfiguration(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::map<int, RefererConfiguration> body_;
};

/// The result for the PutBucketReferer operation.
class ALIBABACLOUD_OSS_API PutBucketRefererResult final : public ResultModel {
  public:
    PutBucketRefererResult() = default;
    PutBucketRefererResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the GetBucketReferer operation.
class ALIBABACLOUD_OSS_API GetBucketRefererRequest final : public RequestModel {
  public:
    GetBucketRefererRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketRefererRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketReferer operation.
class ALIBABACLOUD_OSS_API GetBucketRefererResult final : public ResultModel {
  public:
    GetBucketRefererResult() = default;
    GetBucketRefererResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the hotlink protection configurations.
    inline const RefererConfiguration& getRefererConfiguration() {
        return body_[0];
    }

    inline bool hasRefererConfiguration() const {
        return bodyIsSet_;
    }

    template <typename ValueT = RefererConfiguration>
    GetBucketRefererResult& setRefererConfiguration(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, RefererConfiguration> body_;
    bool bodyIsSet_{};
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
