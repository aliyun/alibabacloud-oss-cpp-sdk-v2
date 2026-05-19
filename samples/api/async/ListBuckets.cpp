// Demonstrates: Async list all buckets owned by the current user.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty())
        sample::printUsageAndExit(argv[0], "");

    auto client = sample::createAsyncClient(args);

    auto future = client->asyncCall(oss::models::ListBucketsRequest());

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListBuckets fail"
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

    for (const auto& b : result.getBuckets()) {
        std::cout << "Bucket: " << b.name
                  << ", Location: " << b.location
                  << ", StorageClass: " << b.storageClass
                  << ", CreationDate: " << b.creationDate << std::endl;
    }
    return 0;
}
