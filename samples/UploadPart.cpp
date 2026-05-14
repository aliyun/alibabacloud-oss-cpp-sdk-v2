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
        printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --upload-id <id> [--part-number <n>]");

    auto client = createClient(args);

    std::string data = "sample part data for upload";
    auto outcome = client.uploadPart(models::UploadPartRequest()
                                             .setBucket(args.bucket)
                                             .setKey(args.key)
                                             .setUploadId(uploadId)
                                             .setPartNumber(partNumber)
                                             .setBody(RequestBody::FromString(data)));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "UploadPart fail, code: " << e.getCode() << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode() << ", requestId: " << result.getRequestId()
              << ", eTag: " << result.getETag() << ", hashCrc64ecma: " << result.getHashCrc64ecma() << std::endl;
    return 0;
}
