// Demonstrates: Use asyncCallback() on OSSClient to run operations with completion callbacks.
//
// asyncCallback() dispatches a sync operation onto the Executor and invokes a user-provided
// callback with signature: (const oss::OSSClient*, const RequestT&, const OutcomeT&).
//
// Usage:
//   ./AsyncCallbackOnSyncClient --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>

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

    // Upload a few objects first (sync, for setup)
    const int count = 3;
    for (int i = 0; i < count; i++) {
        std::string key = "callback-demo/object-" + std::to_string(i);
        std::string data = "Data for callback demo #" + std::to_string(i);
        auto outcome = client.putObject(
            oss::models::PutObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setBody(oss::RequestBody::FromString(data)));
        if (!outcome.has_value()) {
            std::cerr << "Setup PutObject failed for " << key << std::endl;
            return 1;
        }
    }

    // --- Batch HeadObject using asyncCallback ---
    std::atomic<int> pending(count);
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> hasError(false);

    for (int i = 0; i < count; i++) {
        std::string key = "callback-demo/object-" + std::to_string(i);

        client.asyncCallback(
            oss::models::HeadObjectRequest()
                .setBucket(bucket)
                .setKey(key),
            [&pending, &cv, &hasError](
                const oss::OSSClient*,
                const oss::models::HeadObjectRequest& req,
                const oss::HeadObjectOutcome& outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "HeadObject fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage() << std::endl;
                    hasError = true;
                } else {
                    std::cout << "HeadObject(" << req.getKey() << ") done"
                              << ", contentLength: " << outcome.value().getContentLength()
                              << ", contentType: " << outcome.value().getContentType()
                              << std::endl;
                }
                if (--pending == 0) cv.notify_one();
            });
    }

    // Wait for all callbacks to complete
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return pending.load() == 0; });
    }

    // Cleanup
    for (int i = 0; i < count; i++) {
        std::string key = "callback-demo/object-" + std::to_string(i);
        client.deleteObject(
            oss::models::DeleteObjectRequest()
                .setBucket(bucket)
                .setKey(key));
    }

    if (hasError) {
        std::cerr << "Some operations failed." << std::endl;
        return 1;
    }
    std::cout << "All " << count << " asyncCallback operations completed successfully." << std::endl;
    return 0;
}
