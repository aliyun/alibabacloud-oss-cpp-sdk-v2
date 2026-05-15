#pragma once

#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportOptions.h"

#include <cstdlib>
#include <memory>
#include <string>

namespace perf {

constexpr const char* kKeyPrefix = "benchmark-cpp-sdk/";

struct Config {
    std::string accessKeyId;
    std::string accessKeySecret;
    std::string region;
    std::string endpoint;
    std::string bucket;

    int concurrency = 0;            // 0 means not set, BM_*_Custom_Concurrent tests will be skipped
    int objectSize = 1024;           // object size in bytes for BM_*_Custom_Concurrent tests
    unsigned int syncPoolSize = 0;  // 0 means use default
    unsigned int asyncPoolSize = 0; // 0 means use default

    static Config LoadFromEnv() {
        Config cfg;
        auto getEnv = [](const char* name) -> std::string {
            const char* v = std::getenv(name);
            return v ? std::string(v) : std::string();
        };
        cfg.accessKeyId = getEnv("OSS_TEST_ACCESS_KEY_ID");
        cfg.accessKeySecret = getEnv("OSS_TEST_ACCESS_KEY_SECRET");
        cfg.region = getEnv("OSS_TEST_REGION");
        cfg.endpoint = getEnv("OSS_TEST_ENDPOINT");
        cfg.bucket = getEnv("OSS_TEST_BUCKET");
        return cfg;
    }

    bool isValid() const {
        return !accessKeyId.empty() && !accessKeySecret.empty() &&
               !region.empty() && !bucket.empty();
    }
};

inline void ParseCustomArgs(int& argc, char** argv, Config& cfg) {
    int out = 1;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--concurrency" && i + 1 < argc) {
            cfg.concurrency = std::atoi(argv[++i]);
        } else if (arg == "--object_size" && i + 1 < argc) {
            cfg.objectSize = std::atoi(argv[++i]);
        } else if (arg == "--sync_pool_size" && i + 1 < argc) {
            cfg.syncPoolSize = static_cast<unsigned int>(std::atoi(argv[++i]));
        } else if (arg == "--async_pool_size" && i + 1 < argc) {
            cfg.asyncPoolSize = static_cast<unsigned int>(std::atoi(argv[++i]));
        } else {
            argv[out++] = argv[i];
        }
    }
    argc = out;
}

inline Config& GetConfig() {
    static Config cfg = Config::LoadFromEnv();
    return cfg;
}

inline std::shared_ptr<alibabacloud::oss2::OSSClient> GetSyncClient() {
    static std::shared_ptr<alibabacloud::oss2::OSSClient> client;
    if (!client) {
        auto& cfg = GetConfig();
        auto provider = std::make_shared<alibabacloud::oss2::StaticCredentialsProvider>(
            cfg.accessKeyId, cfg.accessKeySecret);
        auto config = alibabacloud::oss2::ClientConfiguration::loadDefault();
        config.region = cfg.region;
        if (!cfg.endpoint.empty()) config.endpoint = cfg.endpoint;
        config.credentialsProvider = provider;

        if (cfg.syncPoolSize > 0) {
            alibabacloud::oss2::CurlTransportOptions transportOpts;
            transportOpts.poolSize = cfg.syncPoolSize;
            config.httpTransport =
                alibabacloud::oss2::CurlTransportFactory::createHttpTransport(transportOpts);
        }

        client = std::make_shared<alibabacloud::oss2::OSSClient>(config);
    }
    return client;
}

inline std::shared_ptr<alibabacloud::oss2::OSSAsyncClient> GetAsyncClient() {
    static std::shared_ptr<alibabacloud::oss2::OSSAsyncClient> client;
    if (!client) {
        auto& cfg = GetConfig();
        auto provider = std::make_shared<alibabacloud::oss2::StaticCredentialsProvider>(
            cfg.accessKeyId, cfg.accessKeySecret);
        auto config = alibabacloud::oss2::ClientConfiguration::loadDefault();
        config.region = cfg.region;
        if (!cfg.endpoint.empty()) config.endpoint = cfg.endpoint;
        config.credentialsProvider = provider;

        if (cfg.asyncPoolSize > 0) {
            alibabacloud::oss2::CurlTransportOptions transportOpts;
            transportOpts.poolSize = cfg.asyncPoolSize;
            config.asyncHttpTransport =
                alibabacloud::oss2::CurlTransportFactory::createAsyncHttpTransport(transportOpts);
        }

        client = std::make_shared<alibabacloud::oss2::OSSAsyncClient>(config);
    }
    return client;
}

} // namespace perf
