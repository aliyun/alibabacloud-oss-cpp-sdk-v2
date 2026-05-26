#include <gtest/gtest.h>

#include <cstdlib>
#ifdef _WIN32
static inline int test_setenv(const char* name, const char* value) { return _putenv_s(name, value); }
static inline int test_unsetenv(const char* name) { return _putenv_s(name, ""); }
#else
static inline int test_setenv(const char* name, const char* value) { return setenv(name, value, 1); }
static inline int test_unsetenv(const char* name) { return unsetenv(name); }
#endif

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

using namespace alibabacloud::oss2;

class EnvCredentialsProviderTest : public ::testing::Test {
  protected:
    void SetUp() override {
        auto* v = std::getenv("OSS_ACCESS_KEY_ID");
        origAk_ = v ? v : "";
        hasAk_ = (v != nullptr);

        v = std::getenv("OSS_ACCESS_KEY_SECRET");
        origSk_ = v ? v : "";
        hasSk_ = (v != nullptr);

        v = std::getenv("OSS_SESSION_TOKEN");
        origToken_ = v ? v : "";
        hasToken_ = (v != nullptr);
    }

    void TearDown() override {
        if (hasAk_) test_setenv("OSS_ACCESS_KEY_ID", origAk_.c_str());
        else test_unsetenv("OSS_ACCESS_KEY_ID");

        if (hasSk_) test_setenv("OSS_ACCESS_KEY_SECRET", origSk_.c_str());
        else test_unsetenv("OSS_ACCESS_KEY_SECRET");

        if (hasToken_) test_setenv("OSS_SESSION_TOKEN", origToken_.c_str());
        else test_unsetenv("OSS_SESSION_TOKEN");
    }

    std::string origAk_, origSk_, origToken_;
    bool hasAk_{false}, hasSk_{false}, hasToken_{false};
};

TEST_F(EnvCredentialsProviderTest, MissingEnvVarsReturnsError) {
    test_unsetenv("OSS_ACCESS_KEY_ID");
    test_unsetenv("OSS_ACCESS_KEY_SECRET");
    test_unsetenv("OSS_SESSION_TOKEN");

    EnvironmentVariableCredentialsProvider provider;
    auto cred = provider.getCredentials();
    EXPECT_TRUE(cred.getError().has_value());
}

TEST_F(EnvCredentialsProviderTest, WithAkSkOnly) {
    test_setenv("OSS_ACCESS_KEY_ID", "test-ak");
    test_setenv("OSS_ACCESS_KEY_SECRET", "test-sk");
    test_unsetenv("OSS_SESSION_TOKEN");

    EnvironmentVariableCredentialsProvider provider;
    auto cred = provider.getCredentials();
    EXPECT_FALSE(cred.getError().has_value());
    EXPECT_EQ("test-ak", cred.getAccessKeyId());
    EXPECT_EQ("test-sk", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
}

TEST_F(EnvCredentialsProviderTest, WithAkSkAndToken) {
    test_setenv("OSS_ACCESS_KEY_ID", "test-ak");
    test_setenv("OSS_ACCESS_KEY_SECRET", "test-sk");
    test_setenv("OSS_SESSION_TOKEN", "test-token");

    EnvironmentVariableCredentialsProvider provider;
    auto cred = provider.getCredentials();
    EXPECT_FALSE(cred.getError().has_value());
    EXPECT_EQ("test-ak", cred.getAccessKeyId());
    EXPECT_EQ("test-sk", cred.getAccessKeySecret());
    EXPECT_EQ("test-token", cred.getSessionToken());
}

TEST_F(EnvCredentialsProviderTest, MissingAkOnly) {
    test_unsetenv("OSS_ACCESS_KEY_ID");
    test_setenv("OSS_ACCESS_KEY_SECRET", "test-sk");

    EnvironmentVariableCredentialsProvider provider;
    auto cred = provider.getCredentials();
    EXPECT_TRUE(cred.getError().has_value());
}

TEST_F(EnvCredentialsProviderTest, MissingSkOnly) {
    test_setenv("OSS_ACCESS_KEY_ID", "test-ak");
    test_unsetenv("OSS_ACCESS_KEY_SECRET");

    EnvironmentVariableCredentialsProvider provider;
    auto cred = provider.getCredentials();
    EXPECT_TRUE(cred.getError().has_value());
}
