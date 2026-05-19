// Demonstrates: Configure hotlink protection (Referer whitelist) for a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    oss::models::RefererList refererList;
    refererList.referers.push_back("https://www.example.com");
    refererList.referers.push_back("https://*.example.com");

    oss::models::RefererConfiguration refConf;
    refConf.allowEmptyReferer = true;
    refConf.refererList = refererList;

    auto outcome = client.putBucketReferer(
        oss::models::PutBucketRefererRequest()
            .setBucket(args.bucket)
            .setRefererConfiguration(refConf));
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
