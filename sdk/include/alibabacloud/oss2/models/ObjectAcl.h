#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {

// The request for the PutObjectAcl operation.
class ALIBABACLOUD_OSS_API PutObjectAclRequest final : public RequestModel {
  public:
    PutObjectAclRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutObjectAclRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    PutObjectAclRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The access control list (ACL) of the object.
    inline const std::string& getObjectAcl() const {
        return getHeaderOrEmpty("x-oss-object-acl");
    }
    template <typename ValueT = std::string>
    PutObjectAclRequest& setObjectAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-object-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The version id of the object.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    PutObjectAclRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the PutObjectAcl operation.
class ALIBABACLOUD_OSS_API PutObjectAclResult final : public ResultModel {
  public:
    PutObjectAclResult() = default;
    PutObjectAclResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // <no value>
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the GetObjectAcl operation.
class ALIBABACLOUD_OSS_API GetObjectAclRequest final : public RequestModel {
  public:
    GetObjectAclRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetObjectAclRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    GetObjectAclRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The verison id of the target object.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    GetObjectAclRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the GetObjectAcl operation.
class ALIBABACLOUD_OSS_API GetObjectAclResult final : public ResultModel {
  public:
    GetObjectAclResult() = default;
    GetObjectAclResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the results of the GetObjectACL request.
    inline const AccessControlPolicy& getAccessControlPolicy() {
        return body_[0];
    }

    inline bool hasAccessControlPolicy() const {
        return bodyIsSet_;
    }

    template <typename ValueT = AccessControlPolicy>
    GetObjectAclResult& setAccessControlPolicy(ValueT&& value) {
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
