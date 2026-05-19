// Demonstrates: Async list objects using callback and future (asyncCall) patterns.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

static void printObjects(const oss::models::ListObjectsV2Result& result) {
    std::cout << ", status: " << result.getStatusCode()
              << ", keyCount: " << result.getKeyCount() << std::endl;
    for (const auto& obj : result.getContents()) {
        std::cout << "  " << obj.key
                  << ", size: " << obj.size
                  << ", lastModified: " << obj.lastModified << std::endl;
    }
}

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    // --- Callback pattern ---
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->listObjectsV2Async(
            oss::models::ListObjectsV2Request()
                .setBucket(args.bucket)
                .setMaxKeys(100),
            oss::ListObjectsV2AsyncCallback([&](oss::ListObjectsV2Outcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "ListObjectsV2(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    std::cout << "ListObjectsV2(callback) done";
                    printObjects(outcome.value());
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
            oss::models::ListObjectsV2Request()
                .setBucket(args.bucket)
                .setMaxKeys(100));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ListObjectsV2(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        std::cout << "ListObjectsV2(future) done";
        printObjects(outcome.value());
    }

    return 0;
}
