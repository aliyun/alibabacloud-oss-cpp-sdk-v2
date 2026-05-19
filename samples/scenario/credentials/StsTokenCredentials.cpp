// Demonstrates: Create an OSSClient with STS temporary credentials.
//
// STS (Security Token Service) provides temporary AK/SK + SecurityToken.
// Obtain these via the STS AssumeRole API or from your application server.
//
// Usage:
//   ./StsTokenCredentials --region <region> --bucket <bucket> \
//       --ak <access-key-id> --sk <access-key-secret> --token <sts-token>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <iostream>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, ak, sk, token;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--ak" && i + 1 < argc) ak = argv[++i];
        else if (a == "--sk" && i + 1 < argc) sk = argv[++i];
        else if (a == "--token" && i + 1 < argc) token = argv[++i];
    }
    if (region.empty() || bucket.empty() || ak.empty() || sk.empty() || token.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket>"
                  << " --ak <access-key-id> --sk <access-key-secret> --token <sts-token>"
                  << std::endl;
        return 1;
    }

    // --- STS temporary credentials: AK + SK + SecurityToken ---
    auto provider = std::make_shared<oss::StaticCredentialsProvider>(ak, sk, token);

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
