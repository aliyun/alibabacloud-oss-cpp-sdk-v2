// Demonstrates: Generate a presigned URL for HeadObject.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    oss::models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(
        oss::models::HeadObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key),
        &options);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "Presign(HeadObject) fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "method: " << result.getMethod()
              << ", expiration: " << result.getExpiration()
              << ", url: " << result.getUrl() << std::endl;
    for (const auto& h : result.getSignedHeaders()) {
        std::cout << "  header: " << h.first << " = " << h.second << std::endl;
    }
    return 0;
}
