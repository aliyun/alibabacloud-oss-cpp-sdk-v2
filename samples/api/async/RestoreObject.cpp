// Demonstrates: Async restore an Archive or Cold Archive object.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    oss::models::RestoreRequest restoreReq;
    restoreReq.days = 1;
    restoreReq.jobParameters = oss::models::JobParameters().setTier("Standard");

    auto future = client->asyncCall(
        oss::models::RestoreObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setRestoreRequest(restoreReq));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "RestoreObject fail"
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
              << ", versionId: " << result.getVersionId() << std::endl;
    return 0;
}
