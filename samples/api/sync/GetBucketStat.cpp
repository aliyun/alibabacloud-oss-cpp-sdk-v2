// Demonstrates: Query storage statistics of a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.getBucketStat(oss::models::GetBucketStatRequest().setBucket(args.bucket));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketStat fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    auto& stat = result.getBucketStat();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    std::cout << "Storage: " << stat.storage.value_or(0)
              << ", ObjectCount: " << stat.objectCount.value_or(0)
              << ", MultipartUploadCount: " << stat.multipartUploadCount.value_or(0)
              << ", StandardStorage: " << stat.standardStorage.value_or(0)
              << ", InfrequentAccessStorage: " << stat.infrequentAccessStorage.value_or(0)
              << ", ArchiveStorage: " << stat.archiveStorage.value_or(0) << std::endl;
    return 0;
}
