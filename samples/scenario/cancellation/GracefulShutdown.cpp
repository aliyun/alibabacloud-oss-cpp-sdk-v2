// Demonstrates: Graceful shutdown combining disableRequest() with CancellationToken.
//
// In production, some requests carry a CancellationToken (e.g., long-running
// uploads with user-level timeout) while others do not. When the application
// needs to shut down or trip a circuit breaker:
//
//   1. Call disableRequest() -- immediately fails all in-flight and queued requests.
//   2. Cancel the token -- wakes up requests stuck in retry backoff (waitFor).
//
// After the disruption resolves, call enableRequest() to resume.
//
// This sample uses OSSClient::asyncCall() (sync client + executor) to launch
// concurrent requests, simulating a realistic multi-request scenario.
//
// Usage:
//   ./GracefulShutdown --region <region> --bucket <bucket>

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

    // Shared CancellationTokenSource for requests that need backoff wake-up
    auto cts = oss::CancellationTokenSource::create();

    std::string data(1 * 1024 * 1024, 'A');  // 1 MB payload

    // --- Launch concurrent requests ---
    // Group A: requests WITH CancellationToken (can be woken from retry backoff)
    // Group B: requests WITHOUT CancellationToken (rely solely on disableRequest)
    std::cout << "Launching concurrent requests..." << std::endl;

    oss::OperationOptions optsWithToken;
    optsWithToken.cancellationToken = cts->getToken();

    std::vector<std::future<oss::PutObjectOutcome>> futures;
    std::vector<std::string> keys;
    std::vector<bool> hasToken;

    for (int i = 0; i < 6; i++) {
        std::string key = "graceful-shutdown-demo/obj-" + std::to_string(i);
        keys.push_back(key);

        if (i < 4) {
            // Group A: with token
            hasToken.push_back(true);
            futures.push_back(client.asyncCall(
                oss::models::PutObjectRequest()
                    .setBucket(bucket)
                    .setKey(key)
                    .setBody(oss::RequestBody::fromString(data)),
                &optsWithToken));
        } else {
            // Group B: without token
            hasToken.push_back(false);
            futures.push_back(client.asyncCall(
                oss::models::PutObjectRequest()
                    .setBucket(bucket)
                    .setKey(key)
                    .setBody(oss::RequestBody::fromString(data))));
        }
    }

    std::cout << "  4 requests with CancellationToken (Group A)" << std::endl;
    std::cout << "  2 requests without CancellationToken (Group B)" << std::endl;

    // --- Simulate shutdown after a short delay ---
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "\n--- Initiating graceful shutdown ---" << std::endl;

    // Step 1: Disable client to reject all remaining/new requests
    std::cout << "Step 1: Disabling client requests..." << std::endl;
    client.disableRequest();

    // Step 2: Cancel the token to wake up any request stuck in retry backoff
    std::cout << "Step 2: Canceling token (wakes backoff waiters)..." << std::endl;
    cts->cancel();

    // --- Collect results ---
    std::cout << "\n--- Collecting results ---" << std::endl;
    int succeeded = 0, failed = 0;
    for (size_t i = 0; i < futures.size(); i++) {
        auto outcome = futures[i].get();
        const char* group = hasToken[i] ? "A" : "B";
        if (!outcome.has_value()) {
            failed++;
            std::cout << "  [" << group << "] " << keys[i] << ": failed"
                      << ", code: " << outcome.error().getCode() << std::endl;
        } else {
            succeeded++;
            std::cout << "  [" << group << "] " << keys[i] << ": succeeded"
                      << ", status: " << outcome.value().getStatusCode() << std::endl;
        }
    }
    std::cout << "Results: " << succeeded << " succeeded, " << failed << " failed" << std::endl;

    // --- Re-enable and verify ---
    std::cout << "\n--- Re-enabling client ---" << std::endl;
    client.enableRequest();

    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey("graceful-shutdown-demo/after-resume.txt")
            .setBody(oss::RequestBody::fromString("resumed")));
    if (!outcome.has_value()) {
        std::cerr << "Post-resume request failed"
                  << ", code: " << outcome.error().getCode()
                  << ", message: " << outcome.error().getMessage() << std::endl;
        return 1;
    }
    std::cout << "Post-resume request succeeded"
              << ", status: " << outcome.value().getStatusCode() << std::endl;

    // --- Cleanup ---
    for (auto& key : keys) {
        client.deleteObject(
            oss::models::DeleteObjectRequest().setBucket(bucket).setKey(key));
    }
    client.deleteObject(
        oss::models::DeleteObjectRequest()
            .setBucket(bucket)
            .setKey("graceful-shutdown-demo/after-resume.txt"));

    std::cout << "\nDone. Graceful shutdown and recovery completed." << std::endl;
    return 0;
}
