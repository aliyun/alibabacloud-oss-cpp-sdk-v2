// Demonstrates: Use asyncCall() on OSSClient to run sync operations in parallel via futures.
//
// OSSClient is a synchronous client, but it provides asyncCall() / asyncCallback()
// template methods that dispatch sync operations onto an Executor (thread pool).
// This requires setting config.executor before creating the client.
//
// Usage:
//   ./AsyncCallOnSyncClient --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"

#include <future>
#include <iostream>
#include <string>
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

    // Key: set executor to enable asyncCall() / asyncCallback() on OSSClient
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    conf.executor = std::make_shared<oss::DefaultExecutor>();

    auto client = oss::OSSClient(conf);

    // --- Parallel upload: launch 3 PutObject calls concurrently ---
    const int count = 3;
    std::vector<std::future<oss::PutObjectOutcome>> putFutures;
    for (int i = 0; i < count; i++) {
        std::string key = "async-demo/object-" + std::to_string(i);
        std::string data = "Hello from asyncCall #" + std::to_string(i);
        putFutures.push_back(client.asyncCall(
            oss::models::PutObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setBody(oss::RequestBody::FromString(data))));
    }

    // Collect put results
    for (int i = 0; i < count; i++) {
        auto outcome = putFutures[i].get();
        std::string key = "async-demo/object-" + std::to_string(i);
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "PutObject(" << key << ") fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage() << std::endl;
            return 1;
        }
        std::cout << "PutObject(" << key << ") done"
                  << ", status: " << outcome.value().getStatusCode() << std::endl;
    }

    // --- Parallel download: launch 3 HeadObject calls concurrently ---
    std::vector<std::future<oss::HeadObjectOutcome>> headFutures;
    for (int i = 0; i < count; i++) {
        std::string key = "async-demo/object-" + std::to_string(i);
        headFutures.push_back(client.asyncCall(
            oss::models::HeadObjectRequest()
                .setBucket(bucket)
                .setKey(key)));
    }

    // Collect head results
    for (int i = 0; i < count; i++) {
        auto outcome = headFutures[i].get();
        std::string key = "async-demo/object-" + std::to_string(i);
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "HeadObject(" << key << ") fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage() << std::endl;
            return 1;
        }
        std::cout << "HeadObject(" << key << ") done"
                  << ", contentLength: " << outcome.value().getContentLength() << std::endl;
    }

    // --- Cleanup ---
    for (int i = 0; i < count; i++) {
        std::string key = "async-demo/object-" + std::to_string(i);
        client.deleteObject(
            oss::models::DeleteObjectRequest()
                .setBucket(bucket)
                .setKey(key));
    }

    std::cout << "All " << count << " async operations completed successfully." << std::endl;
    return 0;
}
