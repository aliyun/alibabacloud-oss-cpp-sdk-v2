#include <gtest/gtest.h>

#include <cstdlib>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

using namespace alibabacloud::oss2;

TEST(CredentialsProviderTest, StaticCredentialsProvider) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    EXPECT_EQ(CredentialsProvider::AuthType::DEFAULT, provider.getAuthType());
    auto cred = provider.getCredentials();
    EXPECT_EQ("ak", cred.getAccessKeyId());
    EXPECT_EQ("sk", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
}

TEST(CredentialsProviderTest, StaticCredentialsProviderWithToken) {
    auto provider = StaticCredentialsProvider("ak", "sk", "token");
    EXPECT_EQ(CredentialsProvider::AuthType::DEFAULT, provider.getAuthType());
    auto cred = provider.getCredentials();
    EXPECT_EQ("ak", cred.getAccessKeyId());
    EXPECT_EQ("sk", cred.getAccessKeySecret());
    EXPECT_EQ("token", cred.getSessionToken());
}

TEST(CredentialsProviderTest, CredentialsProviderFunc) {
    auto provider = CredentialsProviderFunc([]() {
        return Credentials("func-ak", "func-sk", "func-token");
    });
    EXPECT_EQ(CredentialsProvider::AuthType::DEFAULT, provider.getAuthType());
    auto cred = provider.getCredentials();
    EXPECT_EQ("func-ak", cred.getAccessKeyId());
    EXPECT_EQ("func-sk", cred.getAccessKeySecret());
    EXPECT_EQ("func-token", cred.getSessionToken());
}

TEST(CredentialsProviderTest, CredentialsProviderFunc_DynamicUpdate) {
    int callCount = 0;
    auto provider = CredentialsProviderFunc([&callCount]() {
        callCount++;
        return Credentials("ak-" + std::to_string(callCount), "sk", "");
    });
    auto cred1 = provider.getCredentials();
    EXPECT_EQ("ak-1", cred1.getAccessKeyId());
    auto cred2 = provider.getCredentials();
    EXPECT_EQ("ak-2", cred2.getAccessKeyId());
    EXPECT_EQ(2, callCount);
}

TEST(CredentialsProviderTest, AnonymousCredentialsProvider) {
    auto provider = AnonymousCredentialsProvider();
    EXPECT_EQ(CredentialsProvider::AuthType::ANONYMOUS, provider.getAuthType());
    auto cred = provider.getCredentials();
    EXPECT_EQ("", cred.getAccessKeyId());
    EXPECT_EQ("", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
}

