
#pragma once
#include <chrono>
#include <optional>
#include <string>

#include "alibabacloud/oss2/OSS_EXPORTS.h"

namespace alibabacloud {
namespace oss2 {

/// Credentials holds the access key pair and optional STS token used
/// to authenticate requests to OSS. Instances are returned by
/// CredentialsProvider::getCredentials().
///
/// On failure, providers return a Credentials created via withError() or
/// withRetryableError() instead of throwing. The caller checks hasKeys()
/// first; if false, getError() carries the failure reason.
class ALIBABACLOUD_OSS_API Credentials {
  public:
    Credentials() : expiration_(std::chrono::system_clock::time_point::max()) {}

    /// Constructs credentials with an access key pair and optional STS token.
    Credentials(std::string accessKeyId, std::string accessKeySecret, std::string sessionToken = "")
        : accessKeyId_(std::move(accessKeyId)),
          accessKeySecret_(std::move(accessKeySecret)),
          sessionToken_(std::move(sessionToken)),
          expiration_(std::chrono::system_clock::time_point::max()) {}

    /// Constructs credentials with an explicit expiration time (for STS tokens).
    Credentials(std::string accessKeyId, std::string accessKeySecret, std::string sessionToken,
                std::chrono::system_clock::time_point expiration)
        : accessKeyId_(std::move(accessKeyId)),
          accessKeySecret_(std::move(accessKeySecret)),
          sessionToken_(std::move(sessionToken)),
          expiration_(expiration) {}

    /// Creates a failed Credentials with a non-retryable error.
    /// The error message propagates to OperationError::getMessage().
    static Credentials withError(std::string error) {
        Credentials c;
        c.error_ = std::move(error);
        return c;
    }

    /// Creates a failed Credentials with a retryable error.
    /// The SDK will automatically retry the request using the configured retryer.
    static Credentials withRetryableError(std::string error) {
        Credentials c;
        c.error_ = std::move(error);
        c.retryable_ = true;
        return c;
    }

    /// Access Key ID
    inline const std::string& getAccessKeyId() const {
        return accessKeyId_;
    }

    /// Access Key Secret
    inline const std::string& getAccessKeySecret() const {
        return accessKeySecret_;
    }

    /// STS Security Token (empty if not using STS)
    inline const std::string& getSessionToken() const {
        return sessionToken_;
    }

    /// The time the credentials will expire at.
    inline const std::chrono::system_clock::time_point& getExpiration() const {
        return expiration_;
    }

    /// Error message when credential retrieval fails; empty on success.
    inline const std::optional<std::string>& getError() const {
        return error_;
    }

    /// Whether the error is transient and the request should be retried.
    inline bool isErrorRetryable() const {
        return retryable_;
    }

    /// Returns true if both access key ID and secret are non-empty.
    inline bool hasKeys() const {
        return !accessKeyId_.empty() && !accessKeySecret_.empty();
    }

    /// Returns true if the credentials have passed their expiration time.
    inline bool isExpired() const {
        return expiration_ <= std::chrono::system_clock::now();
    }

  private:
    std::string accessKeyId_;
    std::string accessKeySecret_;
    std::string sessionToken_;
    std::chrono::system_clock::time_point expiration_;
    std::optional<std::string> error_;
    bool retryable_{false};
};
} // namespace oss2
} // namespace alibabacloud
