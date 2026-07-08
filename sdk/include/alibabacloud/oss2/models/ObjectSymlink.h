#pragma once

#include "alibabacloud/oss2/Types.h"

#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace models {


// The request for the PutSymlink operation.
class ALIBABACLOUD_OSS_API PutSymlinkRequest final : public RequestModel {
  public:
    PutSymlinkRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutSymlinkRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    PutSymlinkRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The target object to which the symbolic link points. The naming conventions for target objects are the same as
    // those for objects.  - Similar to ObjectName, TargetObjectName must be URL-encoded.   - The target object to which
    // a symbolic link points cannot be a symbolic link.
    inline const std::string& getSymlinkTarget() const {
        return getHeaderOrEmpty("x-oss-symlink-target");
    }
    template <typename ValueT = std::string>
    PutSymlinkRequest& setSymlinkTarget(ValueT&& value) {
        headers_.insert_or_assign("x-oss-symlink-target", std::forward<ValueT>(value));
        return *this;
    }

    // The access control list (ACL) of the object. Default value: default. Valid values:- default: The ACL of the
    // object is the same as that of the bucket in which the object is stored. - private: The ACL of the object is
    // private. Only the owner of the object and authorized users can read and write this object. - public-read: The ACL
    // of the object is public-read. Only the owner of the object and authorized users can read and write this object.
    // Other users can only read the object. Exercise caution when you set the object ACL to this value. -
    // public-read-write: The ACL of the object is public-read-write. All users can read and write this object. Exercise
    // caution when you set the object ACL to this value. For more information about the ACL, see **[ACL](~~100676~~)**.
    inline const std::string& getObjectAcl() const {
        return getHeaderOrEmpty("x-oss-object-acl");
    }
    template <typename ValueT = std::string>
    PutSymlinkRequest& setObjectAcl(ValueT&& value) {
        headers_.insert_or_assign("x-oss-object-acl", std::forward<ValueT>(value));
        return *this;
    }

    // The storage class of the bucket. Default value: Standard.  Valid values:- Standard- IA- Archive- ColdArchive
    inline const std::string& getStorageClass() const {
        return getHeaderOrEmpty("x-oss-storage-class");
    }
    template <typename ValueT = std::string>
    PutSymlinkRequest& setStorageClass(ValueT&& value) {
        headers_.insert_or_assign("x-oss-storage-class", std::forward<ValueT>(value));
        return *this;
    }

    // Specifies whether the PutSymlink operation overwrites the object that has the same name as that of the symbolic
    // link you want to create.   - If the value of **x-oss-forbid-overwrite** is not specified or set to **false**,
    // existing objects can be overwritten by objects that have the same names.   - If the value of
    // **x-oss-forbid-overwrite** is set to **true**, existing objects cannot be overwritten by objects that have the
    // same names. If you specify the **x-oss-forbid-overwrite** request header, the queries per second (QPS)
    // performance of OSS is degraded. If you want to use the **x-oss-forbid-overwrite** request header to perform a
    // large number of operations (QPS greater than 1,000), contact technical support.  The **x-oss-forbid-overwrite**
    // request header is invalid when versioning is enabled or suspended for the destination bucket. In this case, the
    // object with the same name can be overwritten.
    inline const std::string& getForbidOverwrite() const {
        return getHeaderOrEmpty("x-oss-forbid-overwrite");
    }
    template <typename ValueT = std::string>
    PutSymlinkRequest& setForbidOverwrite(ValueT&& value) {
        headers_.insert_or_assign("x-oss-forbid-overwrite", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the PutSymlink operation.
class ALIBABACLOUD_OSS_API PutSymlinkResult final : public ResultModel {
  public:
    PutSymlinkResult() = default;
    PutSymlinkResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The version ID of the object.
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the GetSymlink operation.
class ALIBABACLOUD_OSS_API GetSymlinkRequest final : public RequestModel {
  public:
    GetSymlinkRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetSymlinkRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    GetSymlinkRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version of the object to which the symbolic link points.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    GetSymlinkRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the GetSymlink operation.
class ALIBABACLOUD_OSS_API GetSymlinkResult final : public ResultModel {
  public:
    GetSymlinkResult() = default;
    GetSymlinkResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The target object that the symbolic link points to.
    inline const std::string& getSymlinkTarget() const {
        return getHeaderOrEmpty("x-oss-symlink-target");
    }

    // The version ID of the object.
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
