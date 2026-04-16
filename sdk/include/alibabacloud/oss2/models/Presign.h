#pragma once

#include "alibabacloud/oss2/Types.h"

#include <chrono>
#include <ctime>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace models {

/**
 * @brief Options for presign operations.
 *
 * Controls the expiration time for the generated presigned URL.
 * When expiration is 0 (default), the signer uses a 15-minute default.
 */
class ALIBABACLOUD_OSS_API PresignOptions final {
  public:
    PresignOptions() = default;

    /**
     * @brief Returns the expiration time as an absolute epoch timestamp.
     */
    inline std::time_t getExpiration() const {
        return expiration_;
    }

    /**
     * @brief Returns true if a valid expiration timestamp is set.
     */
    inline bool hasExpiration() const {
        return expiration_ > static_cast<std::time_t>(0);
    }

    /**
     * @brief Sets the expiration time as an absolute epoch timestamp.
     */
    PresignOptions& setExpiration(std::time_t value) {
        expiration_ = value;
        return *this;
    }

    /**
     * @brief Sets the expiration as a duration from now.
     *
     * The minimum precision is seconds.
     */
    PresignOptions& setExpirationDuration(std::chrono::seconds duration) {
        expiration_ = std::time(nullptr) + static_cast<std::time_t>(duration.count());
        return *this;
    }

  private:
    std::time_t expiration_ = 0;
};


// Presign result types
class ALIBABACLOUD_OSS_API PresignResult final {
  public:
    PresignResult() = default;

    inline const std::string& getUrl() const {
        return url_;
    }

    template <typename ValueT = std::string>
    PresignResult& setUrl(ValueT&& value) {
        url_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::string& getMethod() const {
        return method_;
    }

    template <typename ValueT = std::string>
    PresignResult& setMethod(ValueT&& value) {
        method_ = std::forward<ValueT>(value);
        return *this;
    }

    inline std::time_t getExpiration() const {
        return expiration_;
    }

    template <typename ValueT = std::time_t>
    PresignResult& setExpiration(ValueT&& value) {
        expiration_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const HeaderCollection& getSignedHeaders() const {
        return signedHeaders_;
    }

    template <typename ValueT = HeaderCollection>
    PresignResult& setSignedHeaders(ValueT&& value) {
        signedHeaders_ = std::forward<ValueT>(value);
        return *this;
    }

  private:
    std::string url_;
    std::string method_;
    std::time_t expiration_;
    HeaderCollection signedHeaders_;
};


} // namespace models
} // namespace oss2
} // namespace alibabacloud