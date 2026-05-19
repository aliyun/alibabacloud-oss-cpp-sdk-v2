// Demonstrates: Query versioning state of a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.getBucketVersioning(
        oss::models::GetBucketVersioningRequest()
            .setBucket(args.bucket));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketVersioning fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    if (result.hasVersioningConfiguration()) {
        auto& vc = result.getVersioningConfiguration();
        std::cout << "versioning status: " << vc.status.value_or("(not set)") << std::endl;
    }
    return 0;
}
