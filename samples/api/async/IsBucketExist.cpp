// Demonstrates: Async check whether a bucket exists.
//
// Return values:
//   outcome.value() == true  : the bucket exists
//   outcome.value() == false : the bucket does not exist
//   outcome.has_value() == false (error) : unable to determine;
//       this does NOT imply the bucket exists or does not exist.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;

    client->isBucketExistAsync(args.bucket,
        [&](oss::BoolOutcome outcome) {
            if (!outcome.has_value()) {
                auto& e = outcome.error();
                std::cerr << "IsBucketExistAsync error, unable to determine existence"
                          << ", code: " << e.getCode()
                          << ", message: " << e.getMessage()
                          << ", requestId: " << e.getRequestId() << std::endl;
            } else {
                std::cout << "Bucket \"" << args.bucket << "\" "
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
