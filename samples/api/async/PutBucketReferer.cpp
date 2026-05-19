// Demonstrates: Async configure hotlink protection for a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    oss::models::RefererList refererList;
    refererList.referers.push_back("https://www.example.com");
    refererList.referers.push_back("https://*.example.com");

    oss::models::RefererConfiguration refConf;
    refConf.allowEmptyReferer = true;
    refConf.refererList = refererList;

    auto future = client->asyncCall(
        oss::models::PutBucketRefererRequest()
            .setBucket(args.bucket)
            .setRefererConfiguration(refConf));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutBucketReferer fail"
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
    return 0;
}
