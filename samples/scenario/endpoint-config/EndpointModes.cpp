// Demonstrates: Configure various endpoint modes in ClientConfiguration.
//
// The SDK supports multiple endpoint styles:
//   - Custom endpoint: override the auto-derived endpoint
//   - Dual-stack: IPv4 + IPv6 access via a single endpoint
//   - Internal: free traffic between Alibaba Cloud services in the same region
//   - Accelerate: cross-region transfer acceleration
//   - CName: use a custom domain bound to the bucket
//   - Path-style: bucket name in the URL path instead of subdomain
//
// Usage:
//   ./EndpointModes --region <region> --bucket <bucket> --mode <mode>
//   Modes: custom, dual-stack, internal, accelerate, cname, path-style

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, mode = "custom";
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--mode" && i + 1 < argc) mode = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>] [--mode custom|dual-stack|internal|accelerate|cname|path-style]"
                  << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    if (mode == "custom") {
        // Override the auto-derived endpoint with a user-specified one
        if (endpoint.empty()) {
            std::cerr << "custom mode requires --endpoint" << std::endl;
            return 1;
        }
        conf.endpoint = endpoint;
        std::cout << "Mode: custom endpoint = " << endpoint << std::endl;

    } else if (mode == "dual-stack") {
        // IPv4 + IPv6 dual-stack endpoint
        conf.useDualStackEndpoint = true;
        std::cout << "Mode: dual-stack endpoint" << std::endl;

    } else if (mode == "internal") {
        // Internal endpoint (free traffic within same region)
        conf.useInternalEndpoint = true;
        std::cout << "Mode: internal endpoint" << std::endl;

    } else if (mode == "accelerate") {
        // Transfer acceleration endpoint
        conf.useAccelerateEndpoint = true;
        std::cout << "Mode: accelerate endpoint" << std::endl;

    } else if (mode == "cname") {
        // Custom domain (CName) — requires --endpoint as the custom domain
        if (endpoint.empty()) {
            std::cerr << "cname mode requires --endpoint <your-custom-domain>" << std::endl;
            return 1;
        }
        conf.endpoint = endpoint;
        conf.useCName = true;
        std::cout << "Mode: CName = " << endpoint << std::endl;

    } else if (mode == "path-style") {
        // Path-style: https://endpoint/bucket/key instead of https://bucket.endpoint/key
        conf.usePathStyle = true;
        if (!endpoint.empty()) conf.endpoint = endpoint;
        std::cout << "Mode: path-style" << std::endl;

    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

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
    std::cout << "ListObjectsV2 done, keyCount: " << result.getKeyCount()
              << ", requestId: " << result.getRequestId() << std::endl;
    for (const auto& obj : result.getContents()) {
        std::cout << "  " << obj.key << ", size: " << obj.size << std::endl;
    }
    return 0;
}
