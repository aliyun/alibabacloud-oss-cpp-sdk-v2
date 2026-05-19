// Demonstrates: Create a new bucket with storage class and ACL.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.putBucket(
        oss::models::PutBucketRequest()
                    .setBucket(args.bucket)
                    .setAcl("private")
                    .setCreateBucketConfiguration(oss::models::CreateBucketConfiguration().setStorageClass("Standard")));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutBucket fail"
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
