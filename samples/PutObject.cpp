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

    std::string data = "hello world";
    auto body = RequestBody::FromString(data);

    auto outcome = client.putObject(models::PutObjectRequest().setBucket(args.bucket).setKey(args.key).setBody(body));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObject fail, code: " << e.getCode() << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode() << ", requestId: " << result.getRequestId()
              << ", hashCrc64ecma: " << result.getHashCrc64ecma() << ", versionId: " << result.getVersionId()
              << std::endl;
    return 0;
}
