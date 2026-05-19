#pragma once

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace sample {

struct Args {
    std::string region;
    std::string endpoint;
    std::string bucket;
    std::string key;
};

inline void printUsageAndExit(const char* prog, const char* extra) {
    std::cerr << "Usage: " << prog << " --region <region>" << extra
              << " [--endpoint <endpoint>]" << std::endl;
    exit(1);
}

inline Args parseArgs(int argc, char* argv[]) {
    Args a;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--region" && i + 1 < argc) a.region = argv[++i];
        else if (arg == "--endpoint" && i + 1 < argc) a.endpoint = argv[++i];
        else if (arg == "--bucket" && i + 1 < argc) a.bucket = argv[++i];
        else if (arg == "--key" && i + 1 < argc) a.key = argv[++i];
    }
    return a;
}

inline alibabacloud::oss2::OSSClient createClient(const Args& args) {
    auto conf = alibabacloud::oss2::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.credentialsProvider =
        std::make_shared<alibabacloud::oss2::EnvironmentVariableCredentialsProvider>();
    return alibabacloud::oss2::OSSClient(conf);
}

inline std::shared_ptr<alibabacloud::oss2::OSSAsyncClient> createAsyncClient(const Args& args) {
    auto conf = alibabacloud::oss2::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.credentialsProvider =
        std::make_shared<alibabacloud::oss2::EnvironmentVariableCredentialsProvider>();
    return std::make_shared<alibabacloud::oss2::OSSAsyncClient>(conf);
}

} // namespace sample
