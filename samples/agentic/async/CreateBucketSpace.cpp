// Demonstrates: Async create a bucket space under an agentic bucket.
#include <future>

#include "SampleConfig.h"

#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"

namespace oss = alibabacloud::oss2;

static oss::ClientConfiguration makeConfig(const sample::Args& args) {
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.accountId = args.accountId;
    conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    return conf;
}

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.accountId.empty() ||
        args.agenticBucket.empty())
        sample::printUsageAndExit(
            argv[0], " --account-id <accountId> --bucket <spacePrefix> --agentic-bucket <bucket>");

    auto conf = makeConfig(args);

    // The scoped client resolves the logical space prefix into
    // {prefix}-{accountId}-{region}-bs-apsr, so pass the prefix only.
    auto client = oss::agentic::makeAsyncBucketSpaceClient(conf);

    // The bucket space must be created under an agentic bucket, identified by its
    // full name {bucket}-{accountId}-{region}-ab-apsr.
    auto outcome = client.asyncCall(
        oss::models::PutBucketRequest()
            .setBucket(args.bucket)
            .setAgenticBucket(args.agenticBucket + "-" + args.accountId + "-" + args.region +
                              "-ab-apsr")).get();
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
    std::cout << "status code: " << outcome.value().getStatusCode()
              << ", requestId: " << outcome.value().getRequestId() << std::endl;
    return 0;
}
