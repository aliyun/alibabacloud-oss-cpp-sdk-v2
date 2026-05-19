// Demonstrates: Query the ACL of a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.getBucketAcl(oss::models::GetBucketAclRequest().setBucket(args.bucket));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketAcl fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    auto& acp = result.getAccessControlPolicy();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    if (acp.owner) {
        std::cout << "Owner Id: " << acp.owner->id.value_or("") << std::endl;
    }
    if (acp.accessControlList) {
        std::cout << "ACL Grant: " << acp.accessControlList->grant.value_or("") << std::endl;
    }
    return 0;
}
