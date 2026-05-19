// Demonstrates: Create an OSSClient using credentials from environment variables.
//
// This is the recommended approach for most environments.
// Set the following environment variables before running:
//   export OSS_ACCESS_KEY_ID=<your-ak>
//   export OSS_ACCESS_KEY_SECRET=<your-sk>
//   export OSS_SESSION_TOKEN=<your-sts-token>   # optional, for STS
//
// Usage:
//   ./EnvironmentCredentials --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <iostream>

namespace oss = alibabacloud::oss2;

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

    // --- Environment variable credentials ---
    // Reads OSS_ACCESS_KEY_ID, OSS_ACCESS_KEY_SECRET, and optionally OSS_SESSION_TOKEN
    auto provider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

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
