
#pragma once
#include <chrono>
#include <string>

#include "alibabacloud/oss2/OSS_EXPORTS.h"

namespace alibabacloud {
namespace oss2 {
class ALIBABACLOUD_OSS_API Credentials {
  public:
    Credentials() : expiration_(std::chrono::system_clock::time_point::max()) {}

    Credentials(std::string accessKeyId, std::string accessKeySecret, std::string sessionToken = "")
            : accessKeyId_(std::move(accessKeyId)), accessKeySecret_(std::move(accessKeySecret)),
              sessionToken_(std::move(sessionToken)), expiration_(std::chrono::system_clock::time_point::max()) {}

    Credentials(std::string accessKeyId, std::string accessKeySecret, std::string sessionToken,
                std::chrono::system_clock::time_point expiration_)
            : accessKeyId_(std::move(accessKeyId)), accessKeySecret_(std::move(accessKeySecret)),
              sessionToken_(std::move(sessionToken)), expiration_(expiration_) {}

    inline const std::string& getAccessKeyId() const {
        return accessKeyId_;
    }
    inline const std::string& getAccessKeySecret() const {
        return accessKeySecret_;
    }
    inline const std::string& getSessionToken() const {
        return sessionToken_;
    }

    inline const std::chrono::system_clock::time_point& getExpiration() const {
        return expiration_;
    }

    inline bool hasKeys() const {
        return !accessKeyId_.empty() && !accessKeySecret_.empty();
    }

    inline bool isExpired() const {
        return expiration_ <= std::chrono::system_clock::now();
    }

  private:
    std::string accessKeyId_;
    std::string accessKeySecret_;
    std::string sessionToken_;
    std::chrono::system_clock::time_point expiration_;
};
} // namespace oss2
} // namespace alibabacloud
