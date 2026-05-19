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
                  << "  --concurrency <N>      Concurrency for BM_*_Custom_Concurrent tests\n"
                  << "  --object_size <bytes>  Object size for BM_*_Custom_Concurrent tests (default: 1024)\n"
                  << "  --max_conns_sync <N>   Sync max connections (default: 16)\n"
                  << "  --max_conns_async <N>  Async max connections (default: 100)\n";
        return 1;
    }

    std::cout << "Perf config:\n"
              << "  Region:          " << cfg.region << "\n"
              << "  Endpoint:        " << (cfg.endpoint.empty() ? "(auto)" : cfg.endpoint) << "\n"
              << "  Bucket:          " << cfg.bucket << "\n"
              << "  Concurrency:     " << (cfg.concurrency > 0 ? std::to_string(cfg.concurrency) : "(not set)") << "\n"
              << "  Object size:     " << cfg.objectSize << " bytes\n"
              << "  Sync max conns:  " << (cfg.maxConnsSync > 0 ? std::to_string(cfg.maxConnsSync) : "(default)") << "\n"
              << "  Async max conns: " << (cfg.maxConnsAsync > 0 ? std::to_string(cfg.maxConnsAsync) : "(default)") << "\n\n";

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
