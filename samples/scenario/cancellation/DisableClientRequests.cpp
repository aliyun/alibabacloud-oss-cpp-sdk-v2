// Demonstrates: Disable all requests at the client level and re-enable afterward.
//
// disableRequest() immediately cancels all in-flight requests and rejects new
// ones. After the disruption is resolved, call enableRequest() to resume
// normal operation. This is useful for graceful shutdown sequences or circuit
// breaker patterns.
//
// This sample uses asyncCall() to show in-flight requests being interrupted.
//
// Usage:
//   ./DisableClientRequests --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
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

    std::string data(2 * 1024 * 1024, 'X');  // 2 MB payload

    // Step 1: Launch multiple async requests
    std::cout << "Step 1: Launching 4 concurrent async requests..." << std::endl;
    std::vector<std::future<oss::PutObjectOutcome>> futures;
    std::vector<std::string> keys;
    for (int i = 0; i < 4; i++) {
        std::string key = "disable-demo/obj-" + std::to_string(i);
        keys.push_back(key);
        futures.push_back(client.asyncCall(
            oss::models::PutObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setBody(oss::RequestBody::fromString(data))));
    }

    // Step 2: Disable requests while some may still be in-flight
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    std::cout << "Step 2: Disabling client requests..." << std::endl;
    client.disableRequest();

    // Step 3: Collect results -- some may succeed, others fail
    std::cout << "Step 3: Collecting results..." << std::endl;
    int succeeded = 0, failed = 0;
    for (size_t i = 0; i < futures.size(); i++) {
        auto outcome = futures[i].get();
        if (!outcome.has_value()) {
            failed++;
            std::cout << "  " << keys[i] << ": failed"
                      << ", code: " << outcome.error().getCode() << std::endl;
        } else {
            succeeded++;
            std::cout << "  " << keys[i] << ": succeeded"
                      << ", status: " << outcome.value().getStatusCode() << std::endl;
        }
    }
    std::cout << "  " << succeeded << " succeeded, " << failed << " failed" << std::endl;

    // Step 4: New requests fail immediately while disabled
    std::cout << "Step 4: Sending new request while disabled (should fail)..." << std::endl;
    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey("disable-demo/rejected.txt")
            .setBody(oss::RequestBody::fromString("rejected")));
    if (!outcome.has_value()) {
        std::cout << "  Expected failure"
                  << ", code: " << outcome.error().getCode() << std::endl;
    } else {
        std::cerr << "  Unexpected success" << std::endl;
        return 1;
    }

    // Step 5: Re-enable and verify
    std::cout << "Step 5: Re-enabling client requests..." << std::endl;
    client.enableRequest();

    outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey("disable-demo/resumed.txt")
            .setBody(oss::RequestBody::fromString("resumed")));
    if (!outcome.has_value()) {
        std::cerr << "  Unexpected failure after re-enable"
                  << ", code: " << outcome.error().getCode()
                  << ", message: " << outcome.error().getMessage() << std::endl;
        return 1;
    }
    std::cout << "  Success, status: " << outcome.value().getStatusCode() << std::endl;

    // Cleanup
    for (auto& key : keys) {
        client.deleteObject(
            oss::models::DeleteObjectRequest().setBucket(bucket).setKey(key));
    }
    client.deleteObject(
        oss::models::DeleteObjectRequest()
            .setBucket(bucket).setKey("disable-demo/resumed.txt"));

    std::cout << "Done. Client-level disable/enable works correctly." << std::endl;
    return 0;
}
