// Demonstrates: Configure custom retry strategies using Retryer and BackoffDelayer.
//
// The SDK provides several retry building blocks:
//   - StandardRetryer: configurable max attempts + pluggable backoff + error classification
//   - FullJitterBackoff: exponential backoff with full jitter (default)
//   - EqualJitterBackoff: exponential backoff with equal jitter
//   - FixedDelayBackoff: constant delay between retries
//   - NopRetryer: disable retries entirely
//
// Usage:
//   ./CustomRetryStrategy --region <region> --bucket <bucket> --strategy <strategy>
//   Strategies: fixed, equal-jitter, no-retry, max-attempts

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/Retryer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"

#include <chrono>
#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, strategy = "fixed";
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--strategy" && i + 1 < argc) strategy = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--strategy fixed|equal-jitter|no-retry|max-attempts]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    if (strategy == "fixed") {
        // Fixed 500ms delay between retries, up to 5 attempts
        conf.retryer = std::make_shared<oss::StandardRetryer>(
            5, std::make_unique<oss::FixedDelayBackoff>(std::chrono::milliseconds(500)));
        std::cout << "Strategy: FixedDelayBackoff(500ms), maxAttempts=5" << std::endl;

    } else if (strategy == "equal-jitter") {
        // Exponential backoff with equal jitter: base=200ms, max=10s, up to 4 attempts
        conf.retryer = std::make_shared<oss::StandardRetryer>(
            4, std::make_unique<oss::EqualJitterBackoff>(
                   std::chrono::milliseconds(200), std::chrono::seconds(10)));
        std::cout << "Strategy: EqualJitterBackoff(200ms, 10s), maxAttempts=4" << std::endl;

    } else if (strategy == "no-retry") {
        // Disable retries entirely
        conf.retryer = std::make_shared<oss::NopRetryer>();
        std::cout << "Strategy: NopRetryer (no retries)" << std::endl;

    } else if (strategy == "max-attempts") {
        // Simple: just set max attempts, use default FullJitterBackoff
        conf.retryMaxAttempts = 10;
        std::cout << "Strategy: default FullJitterBackoff, maxAttempts=10" << std::endl;

    } else {
        std::cerr << "Unknown strategy: " << strategy << std::endl;
        return 1;
    }

    auto client = oss::OSSClient(conf);

    // Test: list objects
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
    return 0;
}
