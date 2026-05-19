// Demonstrates: Async list objects in a bucket (v1 API).
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createAsyncClient(args);

    auto future = client->asyncCall(
        oss::models::ListObjectsRequest()
            .setBucket(args.bucket)
            .setMaxKeys(100));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjects fail"
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

    for (const auto& obj : result.getContents()) {
        std::cout << "Object: " << obj.key
                  << ", Size: " << obj.size
                  << ", LastModified: " << obj.lastModified << std::endl;
    }
    return 0;
}
