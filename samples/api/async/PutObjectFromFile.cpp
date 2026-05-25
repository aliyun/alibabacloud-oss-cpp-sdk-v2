// Demonstrates: Async upload a local file as an object using callback pattern.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    std::string filePath = "local-file.txt";

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;

    client->putObjectFromFileAsync(
        oss::models::PutObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key),
        filePath,
        [&](oss::PutObjectOutcome outcome) {
            if (!outcome.has_value()) {
                auto& e = outcome.error();
                std::cerr << "PutObjectFromFileAsync fail"
                          << ", code: " << e.getCode()
                          << ", message: " << e.getMessage()
                          << ", requestId: " << e.getRequestId() << std::endl;
            } else {
                auto& result = outcome.value();
                std::cout << "PutObjectFromFileAsync done"
                          << ", status: " << result.getStatusCode()
                          << ", requestId: " << result.getRequestId() << std::endl;
            }
            std::lock_guard<std::mutex> lk(mtx);
            done = true;
            cv.notify_one();
        });

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [&] { return done; });
    return 0;
}
