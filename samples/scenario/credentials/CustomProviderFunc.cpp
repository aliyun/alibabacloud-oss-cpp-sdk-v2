// Demonstrates: Create an OSSClient with a custom credential provider function.
//
// oss::CredentialsProviderFunc wraps a std::function<oss::Credentials()> that is called
// each time credentials are needed. Use this to load credentials from a config
// file, a secrets manager, a remote service, or any custom source.
//
// Usage:
//   ./CustomProviderFunc --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <cstdlib>
#include <iostream>

namespace oss = alibabacloud::oss2;

// Example: load credentials from custom environment variables or a config file
static oss::Credentials loadCredentialsFromCustomSource() {
    // In a real application, you might read from:
    //   - A config file (e.g., ~/.ossrc)
    //   - A secrets manager (e.g., HashiCorp Vault, KMS Secrets Manager)
    //   - A metadata service endpoint
    //   - A database

    // Here we demonstrate reading from custom env vars as a simple example
    const char* ak = std::getenv("MY_APP_ACCESS_KEY_ID");
    const char* sk = std::getenv("MY_APP_ACCESS_KEY_SECRET");
    const char* token = std::getenv("MY_APP_SESSION_TOKEN");

    if (!ak || !sk) {
        // error message will be propagated to OperationError::getMessage()
        return oss::Credentials::withError(
            "MY_APP_ACCESS_KEY_ID and MY_APP_ACCESS_KEY_SECRET must be set");
        // use withRetryableError for transient failures (e.g., network timeout),
        // the SDK will retry fetching credentials automatically.
        // return oss::Credentials::withRetryableError(
        //     "MY_APP_ACCESS_KEY_ID and MY_APP_ACCESS_KEY_SECRET must be set");
    }

    return oss::Credentials(ak, sk, token ? token : "");
}

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>" << std::endl;
        return 1;
    }

    // --- Custom credential provider function ---
    // The function is called each time credentials are needed,
    // so it can implement rotation, refresh, or caching logic.
    auto provider = std::make_shared<oss::CredentialsProviderFunc>(
        loadCredentialsFromCustomSource);

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
