#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


// The request for the PutBucketAcl operation.
class ALIBABACLOUD_OSS_API PutBucketAclRequest final : public RequestModel {
  public:
    PutBucketAclRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutBucketAclRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The ACL that you want to configure or modify for the bucket. The x-oss-acl header is included in PutBucketAcl
    // requests to configure or modify the ACL of the bucket. If this header is not included, the ACL configurations do
    // not take effect.Valid values:*   public-read-write: All users can read and write objects in the bucket. Exercise
    // caution when you set the value to public-read-write.*   public-read: Only the owner and authorized users of the
    // bucket can read and write objects in the bucket. Other users can only read objects in the bucket. Exercise
    // caution when you set the value to public-read.*   private: Only the owner and authorized users of this bucket can
    // read and write objects in the bucket. Other users cannot access objects in the bucket.
    inline const std::string& getAcl() const {
        return getHeaderOrEmpty("x-oss-acl");
    }
    template <typename ValueT = std::string>
    PutBucketAclRequest& setAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-acl", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the PutBucketAcl operation.
class ALIBABACLOUD_OSS_API PutBucketAclResult final : public ResultModel {
  public:
    PutBucketAclResult() = default;
    PutBucketAclResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

// The request for the GetBucketAcl operation.
class ALIBABACLOUD_OSS_API GetBucketAclRequest final : public RequestModel {
  public:
    GetBucketAclRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetBucketAclRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }


  private:
    std::string bucket_;
};

/// The result for the GetBucketAcl operation.
class ALIBABACLOUD_OSS_API GetBucketAclResult final : public ResultModel {
  public:
    GetBucketAclResult() = default;
    GetBucketAclResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the ACL information.
    inline const AccessControlPolicy& getAccessControlPolicy() {
        return body_[0];
    }

    inline bool hasAccessControlPolicy() const {
        return bodyIsSet_;
    }

    template <typename ValueT = AccessControlPolicy>
    GetBucketAclResult& setAccessControlPolicy(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, AccessControlPolicy> body_;
    bool bodyIsSet_{};
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
