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

    // First append
    std::string data1 = "hello";
    auto outcome = client.appendObject(models::AppendObjectRequest()
                                               .setBucket(args.bucket)
                                               .setKey(args.key)
                                               .setPosition(0)
                                               .setBody(RequestBody::FromString(data1)));
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "AppendObject fail, code: " << e.getCode() << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result1 = outcome.getResult();
    std::cout << "status code: " << result1.getStatusCode() << ", requestId: " << result1.getRequestId()
              << ", nextAppendPosition: " << result1.getNextAppendPosition()
              << ", hashCrc64ecma: " << result1.getHashCrc64ecma() << std::endl;

    // Second append
    std::string data2 = " world";
    auto outcome2 = client.appendObject(models::AppendObjectRequest()
                                                .setBucket(args.bucket)
                                                .setKey(args.key)
                                                .setPosition(result1.getNextAppendPosition())
                                                .setBody(RequestBody::FromString(data2)));
    if (!outcome2.isSuccess()) {
        auto& e = outcome2.getError();
        std::cerr << "AppendObject(2) fail, code: " << e.getCode() << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    auto& result2 = outcome2.getResult();
    std::cout << "status code: " << result2.getStatusCode() << ", requestId: " << result2.getRequestId()
              << ", nextAppendPosition: " << result2.getNextAppendPosition()
              << ", hashCrc64ecma: " << result2.getHashCrc64ecma() << std::endl;
    return 0;
}
