// Demonstrates: Async query hotlink protection configuration of a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    auto future = client->asyncCall(oss::models::GetBucketRefererRequest().setBucket(args.bucket));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketReferer fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    auto& conf = result.getRefererConfiguration();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", allowEmptyReferer: " << conf.allowEmptyReferer.value_or(false) << std::endl;
    if (conf.refererList) {
        for (const auto& r : conf.refererList->referers) {
            std::cout << "Referer: " << r << std::endl;
        }
    }
    return 0;
}
