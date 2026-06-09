// Demonstrates: Async submit an async processing task (e.g. video transcoding) for an object.
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

    std::string process = "video/convert,f_mp4|sys/saveas,o_"
        + oss::utils::Base64Encode(targetKey) + ",b_"
        + oss::utils::Base64Encode(args.bucket);

    // --- Callback pattern ---
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->asyncProcessObjectAsync(
            oss::models::AsyncProcessObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key)
                .setProcess(process),
            oss::AsyncProcessObjectAsyncCallback([&](oss::AsyncProcessObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "AsyncProcessObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "AsyncProcessObject(callback) done"
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
            oss::models::AsyncProcessObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key)
                .setProcess(process));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "AsyncProcessObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "AsyncProcessObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", requestId: " << result.getRequestId()
                  << ", body: " << result.getBody() << std::endl;
    }

    return 0;
}
