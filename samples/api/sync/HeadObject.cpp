// Demonstrates: Query object metadata via HEAD request.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    auto outcome = client.headObject(
        oss::models::HeadObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "HeadObject fail"
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
              << ", contentLength: " << result.getContentLength()
              << ", contentType: " << result.getContentType()
              << ", eTag: " << result.getETag()
              << ", lastModified: " << result.getLastModified()
              << ", storageClass: " << result.getStorageClass()
              << ", objectType: " << result.getObjectType()
              << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;
    return 0;
}
