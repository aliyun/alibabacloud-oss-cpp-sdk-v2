// Demonstrates: Async query detailed information about a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    auto future = client->asyncCall(oss::models::GetBucketInfoRequest().setBucket(args.bucket));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketInfo fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    auto& info = result.getBucketInfo();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    std::cout << "Name: " << info.name
              << ", Location: " << info.location
              << ", StorageClass: " << info.storageClass
              << ", CreationDate: " << info.creationDate << std::endl;
    return 0;
}
