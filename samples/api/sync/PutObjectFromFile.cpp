// Demonstrates: Upload a local file as an object.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    std::string filePath = "local-file.txt";

    auto outcome = client.putObjectFromFile(
        oss::models::PutObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key),
        filePath);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObjectFromFile fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "PutObjectFromFile done"
              << ", status: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;
    return 0;
}
