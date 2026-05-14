#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace alibabacloud::oss2;

static void printUsageAndExit(const char* prog, const char* extra) {
    std::cerr << "Usage: " << prog << " --region <region>" << extra << " [--endpoint <endpoint>]" << std::endl;
    exit(1);
}

struct Args {
    std::string region;
    std::string endpoint;
    std::string bucket;
    std::string key;
    // extra fields filled by caller
};

static Args parseCommonArgs(int argc, char* argv[]) {
    Args a;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--region" && i + 1 < argc)
            a.region = argv[++i];
        else if (arg == "--endpoint" && i + 1 < argc)
            a.endpoint = argv[++i];
        else if (arg == "--bucket" && i + 1 < argc)
            a.bucket = argv[++i];
        else if (arg == "--key" && i + 1 < argc)
            a.key = argv[++i];
    }
    return a;
}

static OSSClient createClient(const Args& args) {
    auto conf = ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) {
        conf.endpoint = args.endpoint;
    }
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();
    return OSSClient(conf);
}

int main(int argc, char* argv[]) {
    auto args = parseCommonArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = createClient(args);

    // Step 1: Initiate multipart upload
    auto initOutcome = client.initiateMultipartUpload(
            models::InitiateMultipartUploadRequest().setBucket(args.bucket).setKey(args.key));
    if (!initOutcome.has_value()) {
        std::cerr << "InitiateMultipartUpload fail: " << initOutcome.error().getMessage() << std::endl;
        return 1;
    }
    auto uploadId = initOutcome.value().getUploadId();
    std::cout << "uploadId: " << uploadId << std::endl;

    // Step 2: Upload parts
    std::vector<models::Part> parts;
    std::string data1 = std::string(100 * 1024, 'a'); // 100KB
    std::string data2 = std::string(100 * 1024, 'b'); // 100KB

    auto up1 = client.uploadPart(models::UploadPartRequest()
                                         .setBucket(args.bucket)
                                         .setKey(args.key)
                                         .setUploadId(uploadId)
                                         .setPartNumber(1)
                                         .setBody(RequestBody::FromString(data1)));
    if (!up1.has_value()) {
        std::cerr << "UploadPart 1 fail: " << up1.error().getMessage() << std::endl;
        return 1;
    }
    parts.push_back(models::Part().setETag(up1.value().getETag()).setPartNumber(1));
    std::cout << "Part 1 uploaded, eTag: " << up1.value().getETag() << std::endl;

    auto up2 = client.uploadPart(models::UploadPartRequest()
                                         .setBucket(args.bucket)
                                         .setKey(args.key)
                                         .setUploadId(uploadId)
                                         .setPartNumber(2)
                                         .setBody(RequestBody::FromString(data2)));
    if (!up2.has_value()) {
        std::cerr << "UploadPart 2 fail: " << up2.error().getMessage() << std::endl;
        return 1;
    }
    parts.push_back(models::Part().setETag(up2.value().getETag()).setPartNumber(2));
    std::cout << "Part 2 uploaded, eTag: " << up2.value().getETag() << std::endl;

    // Step 3: Complete multipart upload
    models::CompleteMultipartUpload complete;
    complete.parts = parts;

    auto outcome = client.completeMultipartUpload(models::CompleteMultipartUploadRequest()
                                                          .setBucket(args.bucket)
                                                          .setKey(args.key)
                                                          .setUploadId(uploadId)
                                                          .setCompleteMultipartUpload(complete));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "CompleteMultipartUpload fail, code: " << e.getCode() << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode() << ", requestId: " << result.getRequestId()
              << ", bucket: " << result.getBucket() << ", key: " << result.getKey() << ", eTag: " << result.getETag()
              << ", location: " << result.getLocation() << ", versionId: " << result.getVersionId() << std::endl;
    return 0;
}
