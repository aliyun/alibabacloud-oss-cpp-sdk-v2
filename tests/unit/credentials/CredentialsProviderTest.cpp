#include <gtest/gtest.h>

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

TEST(CredentialsProviderTest, AnonymousCredentialsProvider) {
    auto provider = AnonymousCredentialsProvider();
    EXPECT_EQ(CredentialsProvider::AuthType::ANONYMOUS, provider.getAuthType());
    auto cred = provider.getCredentials();
    EXPECT_EQ("", cred.getAccessKeyId());
    EXPECT_EQ("", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
}