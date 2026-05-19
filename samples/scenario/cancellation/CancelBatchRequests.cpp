// Demonstrates: Cancel multiple concurrent requests with a single CancellationToken.
//
// When multiple asyncCall() operations share the same CancellationToken,
// calling cancel() on the source aborts all of them at once. This is useful
// for batch operations where partial results are not acceptable.
//
// Usage:
//   ./CancelBatchRequests --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/Cancellation.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

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
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    conf.executor = std::make_shared<oss::DefaultExecutor>();

    auto client = oss::OSSClient(conf);

    // Shared cancellation token for all requests
    auto cts = oss::CancellationTokenSource::create();
    oss::OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    // Launch 5 concurrent uploads
    const int count = 5;
    std::string largeData(5 * 1024 * 1024, 'B');  // 5 MB each
    std::vector<std::future<oss::PutObjectOutcome>> futures;

    std::cout << "Launching " << count << " concurrent uploads..." << std::endl;

    for (int i = 0; i < count; i++) {
        std::string key = "batch-cancel-demo/object-" + std::to_string(i);
        futures.push_back(client.asyncCall(
            oss::models::PutObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setBody(oss::RequestBody::FromString(largeData)),
            &opts));
    }

    // Cancel all after a short delay
    std::thread canceler([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Canceling all " << count << " requests..." << std::endl;
        cts->cancel();
    });

    // Collect results
    int succeeded = 0, canceled = 0;
    for (int i = 0; i < count; i++) {
        auto outcome = futures[i].get();
        std::string key = "batch-cancel-demo/object-" + std::to_string(i);
        if (!outcome.has_value()) {
            canceled++;
            std::cout << "  " << key << ": canceled/failed"
                      << ", code: " << outcome.error().getCode() << std::endl;
        } else {
            succeeded++;
            std::cout << "  " << key << ": done"
                      << ", status: " << outcome.value().getStatusCode() << std::endl;
        }
    }

    canceler.join();

    std::cout << "Results: " << succeeded << " succeeded, "
              << canceled << " canceled/failed" << std::endl;

    // Cleanup any objects that were uploaded
    for (int i = 0; i < count; i++) {
        std::string key = "batch-cancel-demo/object-" + std::to_string(i);
        client.deleteObject(
            oss::models::DeleteObjectRequest()
                .setBucket(bucket)
                .setKey(key));
    }

    return 0;
}
