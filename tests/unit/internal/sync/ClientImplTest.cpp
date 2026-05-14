#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/signer/Signer.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/internal/Defaults.h"


namespace alibabacloud {
namespace oss2 {
namespace internal {

class NopSigner : public Signer {
  public:
    bool sign(SigningContext&) override {
        return true;
    }
    std::string getName() const override {
        return "nop";
    };
};

class NopTransport : public HttpTransport {
  public:
    NopTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override {
        return std::error_code();
    }
    std::string getName() const override {
        return "NopTransport";
    }
};
static ClientOptionsFns defaultClientFns;

TEST(ClientImplTest, DefaultConfiguration) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();

    auto client = ClientImpl(config, defaultClientFns);

    // default
    EXPECT_EQ("oss", client.getOptions().product);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    EXPECT_EQ(AddressStyleType::VirtualHosted, client.getOptions().addressStyle);
    EXPECT_NE(nullptr, client.getOptions().retryer);
    EXPECT_EQ("StandardRetryer", client.getOptions().retryer->getName());
    EXPECT_EQ(defaults::MAX_ATTEMPTS, client.getOptions().retryer->getMaxAttempts());

    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("v4", client.getOptions().signer->getName());

    EXPECT_NE(nullptr, client.getOptions().httpTransport);
    EXPECT_EQ(0ULL, client.getOptions().additionalHeaders.size());
}

TEST(ClientImplTest, ConfigSignatureVersion) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("v4", client.getOptions().signer->getName());

    // set to v1
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.signatureVersion = "v1";
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("v1", client.getOptions().signer->getName());

    // set to v4
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.signatureVersion = "v4";
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("v4", client.getOptions().signer->getName());

    // set to any string
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.signatureVersion = "any";
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("v4", client.getOptions().signer->getName());

    // set from signer interface
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.signer = std::make_shared<NopSigner>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().signer);
    EXPECT_EQ("nop", client.getOptions().signer->getName());
}

TEST(ClientImplTest, ConfigEndpoint) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    // internal
    config = ClientConfiguration::loadDefault();
    config.region = "cn-shanghai";
    config.useInternalEndpoint = true;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-shanghai", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-shanghai-internal.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    // accelerate
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.useAccelerateEndpoint = true;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    // dual stack
    config = ClientConfiguration::loadDefault();
    config.region = "cn-shenzhen";
    config.useDualStackEndpoint = true;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-shenzhen", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("cn-shenzhen.oss.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    // set endpoint
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.endpoint = "http://oss-cn-shenzhen.aliyuncs.com";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-shenzhen.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("http", client.getInnerOptions().endpointScheme);

    // set endpoint with port
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.endpoint = "http://oss-cn-shenzhen.aliyuncs.com:8080";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-shenzhen.aliyuncs.com:8080", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("http", client.getInnerOptions().endpointScheme);

    // set endpoint with path & query
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.endpoint = "https://oss-cn-shenzhen.aliyuncs.com:8080/path?key=123";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-shenzhen.aliyuncs.com:8080", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("https", client.getInnerOptions().endpointScheme);

    // disable ssl
    config = ClientConfiguration::loadDefault();
    config.region = "cn-shanghai";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.disableSsl = true;
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-shanghai", client.getOptions().region);
    EXPECT_NE("", client.getOptions().endpoint);
    EXPECT_EQ("oss-cn-shanghai.aliyuncs.com", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("http", client.getInnerOptions().endpointScheme);
}

TEST(ClientImplTest, ConfigAddressStyle) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);
    EXPECT_EQ(AddressStyleType::VirtualHosted, client.getOptions().addressStyle);

    // cname
    config = ClientConfiguration::loadDefault();
    config.region = "cn-shanghai";
    config.useCName = true;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-shanghai", client.getOptions().region);
    EXPECT_EQ(AddressStyleType::CName, client.getOptions().addressStyle);

    // path-style
    config = ClientConfiguration::loadDefault();
    config.region = "cn-shenzhen";
    config.usePathStyle = true;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-shenzhen", client.getOptions().region);
    EXPECT_EQ(AddressStyleType::Path, client.getOptions().addressStyle);

    // ip endpoint
    config = ClientConfiguration::loadDefault();
    config.region = "cn-beijing";
    config.endpoint = "http://127.0.0.1";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("cn-beijing", client.getOptions().region);
    EXPECT_EQ(AddressStyleType::Path, client.getOptions().addressStyle);
    EXPECT_EQ("127.0.0.1", client.getInnerOptions().endpointAuthority);
    EXPECT_EQ("http", client.getInnerOptions().endpointScheme);
}

TEST(ClientImplTest, ConfigAuthMethod) {}

TEST(ClientImplTest, ConfigProduct) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("oss", client.getOptions().product);

    // set product
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    ClientOptionsFns clientFns = {[](ClientOptions& opt) { opt.product = "oss-cloudbox"; }};
    client = ClientImpl(config, clientFns);
    EXPECT_EQ("oss-cloudbox", client.getOptions().product);
}

TEST(ClientImplTest, ConfigRetryer) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().retryer);
    EXPECT_EQ("StandardRetryer", client.getOptions().retryer->getName());
    EXPECT_EQ(defaults::MAX_ATTEMPTS, client.getOptions().retryer->getMaxAttempts());

    // set retryer
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.retryer = std::make_shared<NopRetryer>();
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().retryer);
    EXPECT_EQ("", client.getOptions().retryer->getName());
    EXPECT_EQ(1, client.getOptions().retryer->getMaxAttempts());

    // set MaxAttempts in retryer
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.retryer = std::make_shared<StandardRetryer>(2);
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().retryer);
    EXPECT_EQ("StandardRetryer", client.getOptions().retryer->getName());
    EXPECT_EQ(2, client.getOptions().retryer->getMaxAttempts());

    // set MaxAttempts in configuration
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.retryMaxAttempts = 10;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().retryer);
    EXPECT_EQ("StandardRetryer", client.getOptions().retryer->getName());
    EXPECT_EQ(10, client.getOptions().retryer->getMaxAttempts());
}
TEST(ClientImplTest, ConfigTimeout) {}
TEST(ClientImplTest, ConfigHttpClient) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("oss", client.getOptions().product);
    EXPECT_EQ("cn-hangzhou", client.getOptions().region);

    // set httpclient
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = std::make_shared<NopTransport>();
    client = ClientImpl(config, defaultClientFns);
    EXPECT_NE(nullptr, client.getOptions().httpTransport);
    EXPECT_EQ("NopTransport", client.getOptions().httpTransport->getName());
}
TEST(ClientImplTest, ConfigProxyHost) {}

TEST(ClientImplTest, ConfigUserAgent) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("alibabacloud-cpp-sdk-v2/0.", client.getInnerOptions().userAgent.substr(0, 26));

    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.userAgent = "/my-agent";
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ("alibabacloud-cpp-sdk-v2/0.", client.getInnerOptions().userAgent.substr(0, 26));
    EXPECT_NE(-1, client.getInnerOptions().userAgent.find("/my-agent"));
}

TEST(ClientImplTest, ConfigCrcCheck) {}

TEST(ClientImplTest, ConfigAdditionalHeaders) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ(0ULL, client.getOptions().additionalHeaders.size());

    // set values
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.additionalHeaders = {"key1", "key2"};
    client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ(2ULL, client.getOptions().additionalHeaders.size());
    EXPECT_EQ("key1", client.getOptions().additionalHeaders.at(0));
    EXPECT_EQ("key2", client.getOptions().additionalHeaders.at(1));
}

TEST(ClientImplTest, ConfigFeatureFlags) {
    // default
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    auto client = ClientImpl(config, defaultClientFns);
    EXPECT_EQ(0ULL, client.getOptions().additionalHeaders.size());

    // Default Values
    config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.additionalHeaders = {"key1", "key2"};
    client = ClientImpl(config, defaultClientFns);
    std::cout << client.getOptions().featureFlags << std::endl;
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::CorrectClockSkew));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::AutoDetectMimeType));
    EXPECT_TRUE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckUpload));
    EXPECT_FALSE(client.hasFlag(FeatureFlagsType::EnableCRC64CheckDonwload));
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud