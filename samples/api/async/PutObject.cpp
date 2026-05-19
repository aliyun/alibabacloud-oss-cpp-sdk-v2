// Demonstrates: Async upload using callback and future (asyncCall) patterns.
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
        std::string data = "hello world (callback)";
        auto body = oss::RequestBody::FromString(data);

        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->putObjectAsync(
            oss::models::PutObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key + "-callback")
                .setBody(body),
            oss::PutObjectAsyncCallback([&](oss::PutObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "PutObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "PutObject(callback) done"
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
        std::string data = "hello world (future)";
        auto body = oss::RequestBody::FromString(data);

        auto future = client->asyncCall(
            oss::models::PutObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key + "-future")
                .setBody(body));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "PutObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "PutObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", requestId: " << result.getRequestId() << std::endl;
    }

    return 0;
}
