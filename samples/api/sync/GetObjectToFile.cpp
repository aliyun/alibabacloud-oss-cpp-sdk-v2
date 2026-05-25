// Demonstrates: Download an object to a local file with automatic resume on failure.
//
// If the download is interrupted mid-stream, retries from the last written offset
// using Range requests. Retries indefinitely until the download succeeds; use a
// cancel token in OperationOptions to limit the total elapsed time.
// CRC-64 verification is supported (controlled by EnableCRC64CheckDownload feature flag).
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    std::string filePath = "downloaded-file.dat";

    auto outcome = client.getObjectToFile(
        oss::models::GetObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key),
        filePath);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetObjectToFile fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "GetObjectToFile done"
              << ", status: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", contentLength: " << result.getContentLength() << std::endl;
    return 0;
}
