// Demonstrates: Async upload a single part in a multipart upload.
#include "SampleConfig.h"

#include "alibabacloud/oss2/io/ByteStream.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string uploadId;
    int partNumber = 1;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--upload-id" && i + 1 < argc)
            uploadId = argv[++i];
        else if (a == "--part-number" && i + 1 < argc)
            partNumber = std::stoi(argv[++i]);
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || uploadId.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --upload-id <id> [--part-number <n>]");

    auto client = sample::createAsyncClient(args);

    std::string data = "sample part data for upload";
    auto future = client->asyncCall(
        oss::models::UploadPartRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setPartNumber(partNumber)
            .setBody(oss::RequestBody::fromString(data)));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "UploadPart fail"
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
              << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;
    return 0;
}
