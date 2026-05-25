// Demonstrates: Async check whether an object exists.
//
// Return values:
//   outcome.value() == true  : the object exists
//   outcome.value() == false : the object does not exist
//   outcome.has_value() == false (error) : unable to determine;
//       this does NOT imply the object exists or does not exist.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;

    client->isObjectExistAsync(args.bucket, args.key,
        [&](oss::BoolOutcome outcome) {
            if (!outcome.has_value()) {
                auto& e = outcome.error();
                std::cerr << "IsObjectExistAsync error, unable to determine existence"
                          << ", code: " << e.getCode()
                          << ", message: " << e.getMessage()
                          << ", requestId: " << e.getRequestId() << std::endl;
            } else {
                std::cout << "Object \"" << args.key << "\" "
                          << (outcome.value() ? "exists" : "does not exist") << std::endl;
            }
            std::lock_guard<std::mutex> lk(mtx);
            done = true;
            cv.notify_one();
        });

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [&] { return done; });
    return 0;
}
