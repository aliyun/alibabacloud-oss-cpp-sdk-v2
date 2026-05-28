// Demonstrates: Complete a multipart upload (initiate + upload parts + complete).
#include "SampleConfig.h"

#include "alibabacloud/oss2/io/ByteStream.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    // Step 1: Initiate multipart upload
    auto initOutcome = client.initiateMultipartUpload(
        oss::models::InitiateMultipartUploadRequest()
            .setBucket(args.bucket)
            .setKey(args.key));
    if (!initOutcome.has_value()) {
        std::cerr << "InitiateMultipartUpload fail"
                  << ", code: " << initOutcome.error().getCode()
                  << ", message: " << initOutcome.error().getMessage()
                  << ", ec: " << initOutcome.error().getEC()
                  << ", requestId: " << initOutcome.error().getRequestId()
                  << ", requestTarget: " << initOutcome.error().getRequestTarget() << std::endl;
        return 1;
    }
    auto uploadId = initOutcome.value().getUploadId();
    std::cout << "uploadId: " << uploadId << std::endl;

    // Step 2: Upload parts
    std::vector<oss::models::Part> parts;
    std::string data1 = std::string(100 * 1024, 'a'); // 100KB
    std::string data2 = std::string(100 * 1024, 'b'); // 100KB

    auto up1 = client.uploadPart(
        oss::models::UploadPartRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setPartNumber(1)
            .setBody(oss::RequestBody::fromString(data1)));
    if (!up1.has_value()) {
        std::cerr << "UploadPart 1 fail"
                  << ", code: " << up1.error().getCode()
                  << ", message: " << up1.error().getMessage()
                  << ", ec: " << up1.error().getEC()
                  << ", requestId: " << up1.error().getRequestId()
                  << ", requestTarget: " << up1.error().getRequestTarget() << std::endl;
        return 1;
    }
    parts.push_back(oss::models::Part().setETag(up1.value().getETag()).setPartNumber(1));
    std::cout << "Part 1 uploaded, eTag: " << up1.value().getETag() << std::endl;

    auto up2 = client.uploadPart(
        oss::models::UploadPartRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setPartNumber(2)
            .setBody(oss::RequestBody::fromString(data2)));
    if (!up2.has_value()) {
        std::cerr << "UploadPart 2 fail"
                  << ", code: " << up2.error().getCode()
                  << ", message: " << up2.error().getMessage()
                  << ", ec: " << up2.error().getEC()
                  << ", requestId: " << up2.error().getRequestId()
                  << ", requestTarget: " << up2.error().getRequestTarget() << std::endl;
        return 1;
    }
    parts.push_back(oss::models::Part().setETag(up2.value().getETag()).setPartNumber(2));
    std::cout << "Part 2 uploaded, eTag: " << up2.value().getETag() << std::endl;

    // Step 3: Complete multipart upload
    oss::models::CompleteMultipartUpload complete;
    complete.parts = parts;

    auto outcome = client.completeMultipartUpload(
        oss::models::CompleteMultipartUploadRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setCompleteMultipartUpload(complete));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "CompleteMultipartUpload fail"
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
              << ", bucket: " << result.getBucket()
              << ", key: " << result.getKey()
              << ", eTag: " << result.getETag()
              << ", location: " << result.getLocation()
              << ", versionId: " << result.getVersionId() << std::endl;
    return 0;
}
