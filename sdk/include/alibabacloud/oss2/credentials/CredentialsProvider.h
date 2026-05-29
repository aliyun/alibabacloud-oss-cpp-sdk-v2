
#pragma once

#include "alibabacloud/oss2/credentials/Credentials.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {

/// CredentialsProvider is the interface for supplying credentials to the SDK.
/// Implementations must be safe to call from multiple threads if the client
/// is used concurrently.
class ALIBABACLOUD_OSS_API CredentialsProvider {
  public:
    enum class AuthType { DEFAULT, ANONYMOUS };

    /// Returns credentials for the current request.
    /// On transient failures (network timeout, service unavailable), return
    /// Credentials::withRetryableError() so the SDK retries automatically.
    /// On permanent failures (misconfiguration), return Credentials::withError().
    virtual Credentials getCredentials() = 0;

    /// Returns ANONYMOUS to skip signing entirely.
    virtual AuthType getAuthType() const {
        return AuthType::DEFAULT;
    }
    virtual ~CredentialsProvider() = default;
};


/// StaticCredentialsProvider returns a fixed set of credentials.
/// Use this when the access key pair is known at construction time and
/// does not need to be refreshed.
class ALIBABACLOUD_OSS_API StaticCredentialsProvider final : public CredentialsProvider {
  public:
    StaticCredentialsProvider(std::string accessKeyId, std::string accessKeySecret, std::string sessionToken = "")
        : credentials_(Credentials(std::move(accessKeyId), std::move(accessKeySecret), std::move(sessionToken))) {}


    Credentials getCredentials() override {
        return credentials_;
    }

  private:
    Credentials credentials_;
};

/// EnvironmentVariableCredentialsProvider reads credentials from environment
/// variables OSS_ACCESS_KEY_ID, OSS_ACCESS_KEY_SECRET, and optionally
/// OSS_SESSION_TOKEN on each call to getCredentials().
class ALIBABACLOUD_OSS_API EnvironmentVariableCredentialsProvider final : public CredentialsProvider {
  public:
    Credentials getCredentials() override;
};

/// CredentialsProviderFunc wraps a std::function<Credentials()> to satisfy
/// the CredentialsProvider interface. Use this to integrate custom credential
/// sources (secrets managers, config files, metadata services, etc.) without
/// defining a new class.
class ALIBABACLOUD_OSS_API CredentialsProviderFunc final : public CredentialsProvider {
  public:
    explicit CredentialsProviderFunc(std::function<Credentials()> func) : func_(std::move(func)) {}

    Credentials getCredentials() override {
        return func_();
    }

  private:
    std::function<Credentials()> func_;
};

/// AnonymousCredentialsProvider skips request signing entirely.
/// Use this for accessing public buckets or resources that do not
/// require authentication.
class ALIBABACLOUD_OSS_API AnonymousCredentialsProvider final : public CredentialsProvider {
  public:
    Credentials getCredentials() override {
        return Credentials("", "");
    }
    AuthType getAuthType() const override {
        return AuthType::ANONYMOUS;
    }
};


} // namespace oss2
} // namespace alibabacloud
