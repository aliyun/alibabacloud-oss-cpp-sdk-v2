#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/models/Shared.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


// The request for the PutObjectTagging operation.
class ALIBABACLOUD_OSS_API PutObjectTaggingRequest final : public RequestModel {
  public:
    PutObjectTaggingRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    PutObjectTaggingRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The name of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    PutObjectTaggingRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version id of the target object.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    PutObjectTaggingRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }

    // The request body schema.
    inline const Tagging& getTagging() const {
        return body_.at(0);
    }

    inline bool hasTagging() const {
        return body_.find(0) != body_.end();
    }

    template <typename ValueT = Tagging>
    PutObjectTaggingRequest& setTagging(ValueT&& value) {
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
    std::map<int, Tagging> body_;
};

/// The result for the PutObjectTagging operation.
class ALIBABACLOUD_OSS_API PutObjectTaggingResult final : public ResultModel {
  public:
    PutObjectTaggingResult() = default;
    PutObjectTaggingResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The version ID of the object.
    inline const std::string& getVersionId() const {
        return getHeaderOrEmpty("x-oss-version-id");
    }


  private:
};

// The request for the GetObjectTagging operation.
class ALIBABACLOUD_OSS_API GetObjectTaggingRequest final : public RequestModel {
  public:
    GetObjectTaggingRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    GetObjectTaggingRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    GetObjectTaggingRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The versionID of the object that you want to query.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    GetObjectTaggingRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the GetObjectTagging operation.
class ALIBABACLOUD_OSS_API GetObjectTaggingResult final : public ResultModel {
  public:
    GetObjectTaggingResult() = default;
    GetObjectTaggingResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The container that stores the returned tag of the bucket.
    inline const Tagging& getTagging() {
        return body_[0];
    }

    inline bool hasTagging() const {
        return bodyIsSet_;
    }

    template <typename ValueT = Tagging>
    GetObjectTaggingResult& setTagging(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, Tagging> body_;
    bool bodyIsSet_{};
};

// The request for the DeleteObjectTagging operation.
class ALIBABACLOUD_OSS_API DeleteObjectTaggingRequest final : public RequestModel {
  public:
    DeleteObjectTaggingRequest() = default;

    // The name of the bucket.
    inline const std::string& getBucket() const {
        return bucket_;
    }

    template <typename ValueT = std::string>
    DeleteObjectTaggingRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    // The full path of the object.
    inline const std::string& getKey() const {
        return key_;
    }
    template <typename ValueT = std::string>
    DeleteObjectTaggingRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The version ID of the object that you want to delete.
    inline const std::string& getVersionId() const {
        return getParameterOrEmpty("versionId");
    }
    template <typename ValueT = std::string>
    DeleteObjectTaggingRequest& setVersionId(ValueT&& value) {
        parameters_.insert_or_assign("versionId", std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::string bucket_;
    std::string key_;
};

/// The result for the DeleteObjectTagging operation.
class ALIBABACLOUD_OSS_API DeleteObjectTaggingResult final : public ResultModel {
  public:
    DeleteObjectTaggingResult() = default;
    DeleteObjectTaggingResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


  private:
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
