#include "PerfConfig.h"
#include <benchmark/benchmark.h>
#include <iostream>

int main(int argc, char** argv) {
    auto& cfg = perf::GetConfig();
    perf::ParseCustomArgs(argc, argv, cfg);

    if (!cfg.isValid()) {
        std::cerr << "Performance tests require environment variables:\n"
                  << "  OSS_TEST_ACCESS_KEY_ID\n"
                  << "  OSS_TEST_ACCESS_KEY_SECRET\n"
                  << "  OSS_TEST_REGION\n"
                  << "  OSS_TEST_BUCKET\n"
                  << "  OSS_TEST_ENDPOINT (optional)\n"
                  << "\nCustom options:\n"
                  << "  --sync_pool_size <N>   Sync connection pool size (default: 16)\n"
                  << "  --async_pool_size <N>  Async connection pool size (default: 100)\n";
        return 1;
    }

    std::cout << "Perf config:\n"
              << "  Region:          " << cfg.region << "\n"
              << "  Endpoint:        " << (cfg.endpoint.empty() ? "(auto)" : cfg.endpoint) << "\n"
              << "  Bucket:          " << cfg.bucket << "\n"
              << "  Sync pool size:  " << (cfg.syncPoolSize > 0 ? std::to_string(cfg.syncPoolSize) : "(default)") << "\n"
              << "  Async pool size: " << (cfg.asyncPoolSize > 0 ? std::to_string(cfg.asyncPoolSize) : "(default)") << "\n\n";

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
