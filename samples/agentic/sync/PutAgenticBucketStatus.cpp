// Demonstrates: Update the status of an agentic bucket.
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
    if (args.region.empty() || args.bucket.empty() || args.accountId.empty())
        sample::printUsageAndExit(argv[0], " --account-id <accountId> --bucket <bucket>");

    auto client = createAgenticClient(args);

    auto outcome = client.putAgenticBucketStatus(
        oss::agentic::models::PutAgenticBucketStatusRequest()
            .setBucket(args.bucket)
            .setAgenticBucketStatus(oss::agentic::models::AgenticBucketStatus().setStatus("enabled")));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutAgenticBucketStatus fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    std::cout << "status code: " << outcome.value().getStatusCode()
              << ", requestId: " << outcome.value().getRequestId() << std::endl;
    return 0;
}
