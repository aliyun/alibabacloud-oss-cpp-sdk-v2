// Demonstrates: Cancel an in-flight request from another thread.
//
// CancellationTokenSource::cancel() immediately marks the token as canceled.
// Any ongoing HTTP operation using that token will be interrupted and return
// an error. This is useful for user-initiated abort (e.g., a "Cancel" button).
//
// Usage:
//   ./CancelInflightRequest --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

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

    auto client = oss::OSSClient(conf);

    // Create a CancellationTokenSource
    auto cts = oss::CancellationTokenSource::create();

    // Spawn a thread that cancels the request after 50ms
    std::thread canceler([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "Canceling the request..." << std::endl;
        cts->cancel();
    });

    // Start a large upload that will be interrupted
    std::string data(10 * 1024 * 1024, 'X');  // 10 MB
    std::string key = "cancel-demo/inflight.dat";

    oss::OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(oss::RequestBody::FromString(data)),
        &opts);

    canceler.join();

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cout << "Request was canceled (expected)"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
    } else {
        std::cout << "Request completed before cancellation"
                  << ", status: " << outcome.value().getStatusCode() << std::endl;
        // Cleanup if upload succeeded
        client.deleteObject(
            oss::models::DeleteObjectRequest()
                .setBucket(bucket)
                .setKey(key));
    }

    return 0;
}
