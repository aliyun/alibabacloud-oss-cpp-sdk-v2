// Demonstrates: Copy an object within or across buckets.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string sourceBucket, sourceKey;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--source-bucket" && i + 1 < argc)
            sourceBucket = argv[++i];
        else if (a == "--source-key" && i + 1 < argc)
            sourceKey = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || sourceBucket.empty() || sourceKey.empty())
        sample::printUsageAndExit(argv[0],
                          " --bucket <bucket> --key <key> --source-bucket <src-bucket> --source-key <src-key>");

    auto client = sample::createClient(args);

    auto outcome = client.copyObject(
        oss::models::CopyObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setSourceBucket(sourceBucket)
            .setSourceKey(sourceKey));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "CopyObject fail"
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
              << ", eTag: " << result.getETag()
              << ", lastModified: " << result.getLastModified()
              << ", versionId: " << result.getVersionId()
              << ", copySourceVersionId: " << result.getCopySourceVersionId() << std::endl;
    return 0;
}
