// Demonstrates: Async append data to an object.
#include "SampleConfig.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    // --- Callback pattern ---
    {
        std::string data = "hello (callback)";
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->appendObjectAsync(
            oss::models::AppendObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key + "-callback")
                .setPosition(0)
                .setBody(oss::RequestBody::fromString(data)),
            oss::AppendObjectAsyncCallback([&](oss::AppendObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "AppendObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "AppendObject(callback) done"
                              << ", status: " << result.getStatusCode()
                              << ", nextAppendPosition: " << result.getNextAppendPosition() << std::endl;
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
        std::string data = "hello (future)";
        auto future = client->asyncCall(
            oss::models::AppendObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key + "-future")
                .setPosition(0)
                .setBody(oss::RequestBody::fromString(data)));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "AppendObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "AppendObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", nextAppendPosition: " << result.getNextAppendPosition()
                  << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;
    }

    return 0;
}
