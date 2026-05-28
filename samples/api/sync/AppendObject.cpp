// Demonstrates: Upload by appending data to an existing object.
#include "SampleConfig.h"

#include "alibabacloud/oss2/io/ByteStream.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    // First append
    std::string data1 = "hello";
    auto outcome = client.appendObject(
        oss::models::AppendObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setPosition(0)
            .setBody(oss::RequestBody::fromString(data1)));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "AppendObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result1 = outcome.value();
    std::cout << "status code: " << result1.getStatusCode()
              << ", requestId: " << result1.getRequestId()
              << ", nextAppendPosition: " << result1.getNextAppendPosition()
              << ", hashCrc64ecma: " << result1.getHashCrc64ecma() << std::endl;

    // Second append
    std::string data2 = " world";
    auto outcome2 = client.appendObject(
        oss::models::AppendObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setPosition(result1.getNextAppendPosition())
            .setBody(oss::RequestBody::fromString(data2)));
    if (!outcome2.has_value()) {
        auto& e = outcome2.error();
        std::cerr << "AppendObject(2) fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    auto& result2 = outcome2.value();
    std::cout << "status code: " << result2.getStatusCode()
              << ", requestId: " << result2.getRequestId()
              << ", nextAppendPosition: " << result2.getNextAppendPosition()
              << ", hashCrc64ecma: " << result2.getHashCrc64ecma() << std::endl;
    return 0;
}
