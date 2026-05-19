// Demonstrates: Async download using callback and future (asyncCall) patterns.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>
#include <sstream>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    // --- Callback pattern ---
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        client->getObjectAsync(
            oss::models::GetObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key),
            oss::GetObjectAsyncCallback([&](oss::GetObjectOutcome outcome) {
                if (!outcome.has_value()) {
                    auto& e = outcome.error();
                    std::cerr << "GetObject(callback) fail"
                              << ", code: " << e.getCode()
                              << ", message: " << e.getMessage()
                              << ", ec: " << e.getEC()
                              << ", requestId: " << e.getRequestId()
                              << ", requestTarget: " << e.getRequestTarget() << std::endl;
                } else {
                    auto& result = outcome.value();
                    std::cout << "GetObject(callback) done"
                              << ", status: " << result.getStatusCode()
                              << ", contentLength: " << result.getContentLength() << std::endl;
                    auto& body = result.getBody();
                    if (body) {
                        std::stringstream ss;
                        ss << body->rdbuf();
                        std::cout << "Content: " << ss.str() << std::endl;
                    }
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
            oss::models::GetObjectRequest()
                .setBucket(args.bucket)
                .setKey(args.key));

        auto outcome = future.get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "GetObject(future) fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC()
                      << ", requestId: " << e.getRequestId()
                      << ", requestTarget: " << e.getRequestTarget() << std::endl;
            return 1;
        }

        auto& result = outcome.value();
        std::cout << "GetObject(future) done"
                  << ", status: " << result.getStatusCode()
                  << ", contentLength: " << result.getContentLength() << std::endl;

        auto& body = result.getBody();
        if (body) {
            std::stringstream ss;
            ss << body->rdbuf();
            std::cout << "Content: " << ss.str() << std::endl;
        }
    }

    return 0;
}
