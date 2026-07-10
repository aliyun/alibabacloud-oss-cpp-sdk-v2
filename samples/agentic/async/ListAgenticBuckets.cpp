// Demonstrates: Async list the agentic buckets owned by the requester.
#include <future>
#include "SampleConfig.h"
#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"

namespace oss = alibabacloud::oss2;

static oss::agentic::OSSAsyncAgenticBucketClient createAsyncAgenticClient(const sample::Args& args) {
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.accountId = args.accountId;
    conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    return oss::agentic::OSSAsyncAgenticBucketClient(conf);
}

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.accountId.empty())
        sample::printUsageAndExit(argv[0], " --account-id <accountId>");

    auto client = createAsyncAgenticClient(args);

    std::promise<oss::agentic::ListAgenticBucketsOutcome> promise;
    client.listAgenticBucketsAsync(
        oss::agentic::models::ListAgenticBucketsRequest(),
        [&promise](oss::agentic::ListAgenticBucketsOutcome outcome) { promise.set_value(std::move(outcome)); });

    auto outcome = promise.get_future().get();
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
