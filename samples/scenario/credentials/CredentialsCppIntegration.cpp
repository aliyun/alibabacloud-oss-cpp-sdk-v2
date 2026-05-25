// Demonstrates: Bridge alibabacloud-credentials-cpp with the OSS SDK via oss::CredentialsProviderFunc.
//
// The Alibaba Cloud Credentials library (https://github.com/aliyun/credentials-cpp) supports
// many credential types: AK, STS, RAM Role ARN, ECS RAM Role, OIDC, URI, Bearer Token, etc.
// This sample shows how to wrap it as an OSS CredentialsProvider.
//
// Prerequisites:
//   - Install alibabacloud-credentials-cpp
//   - Build with: cmake -B build -DCMAKE_PREFIX_PATH="<oss-sdk-prefix>;<credentials-prefix>"
//
// Usage:
//   ./CredentialsCppIntegration --region <region> --bucket <bucket> --cred-type <type>
//
// Supported --cred-type values:
//   access_key       - Use AK/SK from environment (ALIBABA_CLOUD_ACCESS_KEY_ID, etc.)
//   ecs_ram_role     - Use ECS instance RAM role (run on ECS only)
//   ram_role_arn     - Use RAM role ARN (assumes a role via STS)
//   oidc_role_arn    - Use OIDC provider to assume a role
//   default          - Auto-detect (default credentials chain)

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <darabonba/core/Client.hpp>
#include <alibabacloud/credential.hpp>

#include <iostream>
#include <memory>

namespace oss = alibabacloud::oss2;

static std::shared_ptr<Alibabacloud_Credential::Client>
createCredentialClient(const std::string& credType) {
    auto credConfig = std::make_shared<Alibabacloud_Credential::Config>();
    credConfig->type = std::make_shared<std::string>(credType);

    // Type-specific configuration
    // For access_key: reads ALIBABA_CLOUD_ACCESS_KEY_ID / ALIBABA_CLOUD_ACCESS_KEY_SECRET from env
    // For ecs_ram_role: optionally set roleName
    // For ram_role_arn: requires roleArn, roleSessionName, and a source credential
    // For oidc_role_arn: requires oidcProviderArn, oidcTokenFilePath, roleArn, roleSessionName

    if (credType == "ecs_ram_role") {
        // credConfig->roleName = std::make_shared<std::string>("my-ecs-role");
    } else if (credType == "ram_role_arn") {
        // credConfig->accessKeyId = std::make_shared<std::string>("...");
        // credConfig->accessKeySecret = std::make_shared<std::string>("...");
        // credConfig->roleArn = std::make_shared<std::string>("acs:ram::123456:role/my-role");
        // credConfig->roleSessionName = std::make_shared<std::string>("session-name");
    } else if (credType == "oidc_role_arn") {
        // credConfig->roleArn = std::make_shared<std::string>("acs:ram::123456:role/my-role");
        // credConfig->oidcProviderArn = std::make_shared<std::string>("acs:ram::123456:oidc-provider/my-provider");
        // credConfig->oidcTokenFilePath = std::make_shared<std::string>("/var/run/secrets/token");
        // credConfig->roleSessionName = std::make_shared<std::string>("session-name");
    }

    return std::make_shared<Alibabacloud_Credential::Client>(credConfig);
}

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, credType = "default";
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--cred-type" && i + 1 < argc) credType = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket> [--cred-type <type>]" << std::endl;
        return 1;
    }

    // 1. Create the Alibaba Cloud Credentials client
    auto credClient = createCredentialClient(credType);

    std::cout << "Using credential type: " << credType << std::endl;

    // 2. Bridge to OSS SDK via oss::CredentialsProviderFunc
    //    Use getCredential() to get a consistent snapshot of AK/SK/Token,
    //    avoiding misalignment when credentials refresh between individual getter calls.
    auto provider = std::make_shared<oss::CredentialsProviderFunc>(
        [credClient]() -> oss::Credentials {
            auto cred = credClient->getCredential();
            auto ak = cred.getAccessKeyId();
            auto sk = cred.getAccessKeySecret();
            if (ak.empty() || sk.empty()) {
                return oss::Credentials::withRetryableError(
                    "failed to get credentials from alibabacloud-credentials-cpp");
            }
            auto token = cred.getSecurityToken();
            return oss::Credentials(std::move(ak), std::move(sk), std::move(token));
        });

    // 3. Create OSSClient with the bridged provider
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider = provider;

    auto client = oss::OSSClient(conf);

    // Verify: list objects
    auto outcome = client.listObjectsV2(
        oss::models::ListObjectsV2Request()
            .setBucket(bucket)
            .setMaxKeys(5));

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectsV2 fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "ListObjectsV2 done, keyCount: " << result.getKeyCount() << std::endl;
    for (const auto& obj : result.getContents()) {
        std::cout << "  " << obj.key << ", size: " << obj.size << std::endl;
    }
    return 0;
}
