// Demonstrates: List objects in a bucket (v2 API, recommended).
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.listObjectsV2(
        oss::models::ListObjectsV2Request()
            .setBucket(args.bucket)
            .setMaxKeys(100));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectsV2 fail"
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
              << ", keyCount: " << result.getKeyCount()
              << ", isTruncated: " << result.getIsTruncated() << std::endl;

    for (const auto& obj : result.getContents()) {
        std::cout << "Object: " << obj.key
                  << ", Size: " << obj.size
                  << ", LastModified: " << obj.lastModified
                  << ", ETag: " << obj.eTag
                  << ", StorageClass: " << obj.storageClass << std::endl;
    }
    return 0;
}
