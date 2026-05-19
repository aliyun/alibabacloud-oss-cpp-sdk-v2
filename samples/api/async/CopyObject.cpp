// Demonstrates: Async copy an object within or across buckets.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string sourceBucket, sourceKey;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--source-bucket" && i + 1 < argc)
            sourceBucket = argv[++i];
        else if (a == "--source-key" && i + 1 < argc)
            sourceKey = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || sourceBucket.empty() || sourceKey.empty())
        sample::printUsageAndExit(argv[0],
                          " --bucket <bucket> --key <key> --source-bucket <src-bucket> --source-key <src-key>");

    auto client = sample::createAsyncClient(args);

    // --- Callback pattern ---
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->copyObjectAsync(
            oss::models::CopyObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key)
                .setSourceBucket(sourceBucket)
                .setSourceKey(sourceKey),
            oss::CopyObjectAsyncCallback([&](oss::CopyObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "CopyObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "CopyObject(callback) done"
                              << ", status: " << result.getStatusCode()
                              << ", requestId: " << result.getRequestId() << std::endl;
                }
                std::lock_guard<std::mutex> lk(mtx);
                done = true;
                cv.notify_one();
            }));

        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return done; });
    }

    // --- Future pattern (asyncCall) ---
    {
        auto future = client->asyncCall(
            oss::models::CopyObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key + "-future")
                .setSourceBucket(sourceBucket)
                .setSourceKey(sourceKey));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "CopyObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "CopyObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", requestId: " << result.getRequestId()
                  << ", eTag: " << result.getETag()
                  << ", versionId: " << result.getVersionId() << std::endl;
    }

    return 0;
}
