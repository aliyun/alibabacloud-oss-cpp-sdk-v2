
#pragma once

#include "alibabacloud/oss2/credentials/Credentials.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API CredentialsProvider {
  public:
    enum class AuthType { DEFAULT, ANONYMOUS };
    virtual Credentials getCredentials() = 0;
    virtual AuthType getAuthType() {
        return AuthType::DEFAULT;
    };
    virtual ~CredentialsProvider() = default;
};


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

class ALIBABACLOUD_OSS_API EnvironmentVariableCredentialsProvider final : public CredentialsProvider {
  public:
    Credentials getCredentials() override;
};

class ALIBABACLOUD_OSS_API CredentialsProviderFunc final : public CredentialsProvider {
  public:
    explicit CredentialsProviderFunc(std::function<Credentials()> func)
            : func_(std::move(func)) {}

    Credentials getCredentials() override {
        return func_();
    }

  private:
    std::function<Credentials()> func_;
};

class ALIBABACLOUD_OSS_API AnonymousCredentialsProvider final : public CredentialsProvider {
  public:
    Credentials getCredentials() override {
        return Credentials("", "");
    }
    AuthType getAuthType() override {
        return AuthType::ANONYMOUS;
    };
};


} // namespace oss2
} // namespace alibabacloud
