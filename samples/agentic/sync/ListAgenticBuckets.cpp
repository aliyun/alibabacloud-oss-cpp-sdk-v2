// Demonstrates: List the agentic buckets owned by the requester.
#include "SampleConfig.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

namespace oss = alibabacloud::oss2;

static oss::agentic::OSSAgenticBucketClient createAgenticClient(const sample::Args& args) {
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.accountId = args.accountId;
    conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    return oss::agentic::OSSAgenticBucketClient(conf);
}

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.accountId.empty())
        sample::printUsageAndExit(argv[0], " --account-id <accountId>");

    auto client = createAgenticClient(args);

    auto outcome = client.listAgenticBuckets(oss::agentic::models::ListAgenticBucketsRequest());
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListAgenticBuckets fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", isTruncated: " << result.getIsTruncated() << std::endl;
    for (auto& b : result.getAgenticBuckets()) {
        std::cout << "  name: " << b.name.value_or("(not set)")
                  << ", storageClass: " << b.storageClass.value_or("(not set)")
                  << ", createTime: " << b.createTime.value_or("(not set)") << std::endl;
    }
    return 0;
}
