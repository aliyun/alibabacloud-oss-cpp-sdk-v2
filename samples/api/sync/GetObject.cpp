// Demonstrates: Download an object and read its content.
#include "SampleConfig.h"

#include <sstream>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    auto outcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetObject fail"
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
              << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;

    // Read body
    auto& body = result.getBody();
    if (body) {
        std::stringstream ss;
        ss << body->rdbuf();
        std::cout << "Content: " << ss.str() << std::endl;
    }
    return 0;
}
