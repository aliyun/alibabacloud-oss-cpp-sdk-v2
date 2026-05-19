// Demonstrates: Upload a string as an object.
#include "SampleConfig.h"

#include "alibabacloud/oss2/io/ByteStream.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    std::string data = "hello world";
    auto body = oss::RequestBody::FromString(data);

    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setBody(body));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObject fail"
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
              << ", hashCrc64ecma: " << result.getHashCrc64ecma()
              << ", versionId: " << result.getVersionId() << std::endl;
    return 0;
}
