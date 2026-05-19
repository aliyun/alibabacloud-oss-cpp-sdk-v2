// Demonstrates: Set a total timeout for a single request using CancellationToken.
//
// cancelAfter(milliseconds) sets a deadline: if the entire operation (including
// retries) hasn't completed by then, it is automatically canceled.
//
// This is different from connectTimeout/readWriteTimeout which are per-TCP-operation
// timeouts. CancellationToken provides a request-level overall deadline.
//
// Usage:
//   ./RequestTimeout --region <region> --bucket <bucket> [--timeout-ms <ms>]

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <chrono>
#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    long timeoutMs = 5000;  // default 5 seconds
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--timeout-ms" && i + 1 < argc) timeoutMs = std::stol(argv[++i]);
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--timeout-ms <ms>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = oss::OSSClient(conf);

    // Create a CancellationTokenSource with a deadline
    auto cts = oss::CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::milliseconds(timeoutMs));

    std::cout << "Listing objects with " << timeoutMs << "ms total timeout..." << std::endl;

    oss::OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client.listObjectsV2(
        oss::models::ListObjectsV2Request()
            .setBucket(bucket)
            .setMaxKeys(10),
        &opts);

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectsV2 fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "ListObjectsV2 done, keyCount: " << result.getKeyCount()
              << ", requestId: " << result.getRequestId() << std::endl;
    return 0;
}
