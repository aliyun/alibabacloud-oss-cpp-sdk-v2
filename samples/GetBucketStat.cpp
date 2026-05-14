#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
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
    if (args.region.empty() || args.bucket.empty())
        printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = createClient(args);

    auto outcome = client.getBucketStat(models::GetBucketStatRequest().setBucket(args.bucket));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetBucketStat fail, code: " << e.getCode() << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    auto& stat = result.getBucketStat();
    std::cout << "status code: " << result.getStatusCode() << ", requestId: " << result.getRequestId() << std::endl;
    std::cout << "Storage: " << stat.storage.value_or(0) << ", ObjectCount: " << stat.objectCount.value_or(0)
              << ", MultipartUploadCount: " << stat.multipartUploadCount.value_or(0)
              << ", StandardStorage: " << stat.standardStorage.value_or(0)
              << ", InfrequentAccessStorage: " << stat.infrequentAccessStorage.value_or(0)
              << ", ArchiveStorage: " << stat.archiveStorage.value_or(0) << std::endl;
    return 0;
}
