// Demonstrates: Async process an object (e.g. image resize, watermark) and save the result.
#include "SampleConfig.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string targetKey;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--target-key" && i + 1 < argc)
            targetKey = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || targetKey.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --target-key <target-key>");

    auto client = sample::createAsyncClient(args);

    std::string process = "image/resize,w_100|sys/saveas,o_"
        + oss::utils::Base64Encode(targetKey) + ",b_"
        + oss::utils::Base64Encode(args.bucket);

    // --- Callback pattern ---
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->processObjectAsync(
            oss::models::ProcessObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key)
                .setProcess(process),
            oss::ProcessObjectAsyncCallback([&](oss::ProcessObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "ProcessObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "ProcessObject(callback) done"
                              << ", status: " << result.getStatusCode()
                              << ", requestId: " << result.getRequestId()
                              << ", body: " << result.getBody() << std::endl;
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
            oss::models::ProcessObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key)
                .setProcess(process));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ProcessObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "ProcessObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", requestId: " << result.getRequestId()
                  << ", body: " << result.getBody() << std::endl;
    }

    return 0;
}
