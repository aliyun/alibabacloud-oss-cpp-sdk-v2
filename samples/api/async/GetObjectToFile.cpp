// Demonstrates: Async download an object to a local file using callback pattern.
//
// This is a convenience wrapper around getObjectAsync that writes the response body
// to a file. Unlike the synchronous OSSClient::getObjectToFile which performs resumable
// retry from the last written offset, retries here are handled internally by the SDK
// at the request level (each retry downloads from the beginning).
// CRC-64 verification is supported (controlled by EnableCRC64CheckDownload feature flag).
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    std::string filePath = "downloaded-file.dat";

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;

    client->getObjectToFileAsync(
        oss::models::GetObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key),
        filePath,
        [&](oss::GetObjectOutcome outcome) {
            if (!outcome.has_value()) {
                auto& e = outcome.error();
                std::cerr << "GetObjectToFileAsync fail"
                          << ", code: " << e.getCode()
                          << ", message: " << e.getMessage()
                          << ", requestId: " << e.getRequestId() << std::endl;
            } else {
                auto& result = outcome.value();
                std::cout << "GetObjectToFileAsync done"
                          << ", status: " << result.getStatusCode()
                          << ", requestId: " << result.getRequestId()
                          << ", contentLength: " << result.getContentLength() << std::endl;
            }
            std::lock_guard<std::mutex> lk(mtx);
            done = true;
            cv.notify_one();
        });

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [&] { return done; });
    return 0;
}
