#include "PerfConfig.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <benchmark/benchmark.h>
#include <future>
#include <string>
#include <vector>

using namespace alibabacloud::oss2;

static const char* kGetKey1KB = "perf-get-fixture-1k.dat";
static const char* kGetKey1MB = "perf-get-fixture-1m.dat";
static const char* kGetKey4MB = "perf-get-fixture-4m.dat";

static void ensureFixtureObjects() {
    static bool done = false;
    if (done) return;

    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    auto body1k = RequestBody::FromString(std::string(1024, 'R'));
    client->putObject(
        models::PutObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1KB).setBody(body1k));

    auto body1m = RequestBody::FromString(std::string(1024 * 1024, 'S'));
    client->putObject(
        models::PutObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1MB).setBody(body1m));

    auto body4m = RequestBody::FromString(std::string(4 * 1024 * 1024, 'T'));
    client->putObject(
        models::PutObjectRequest().setBucket(cfg.bucket).setKey(kGetKey4MB).setBody(body4m));

    done = true;
}

static void BM_GetObject_Sync_1KB(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1KB));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_GetObject_Sync_1KB)->Iterations(50)->UseRealTime();

static void BM_GetObject_Sync_1MB(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1MB));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Sync_1MB)->Iterations(20)->UseRealTime();

static void BM_GetObject_Sync_4MB(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey4MB));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 4 * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Sync_4MB)->Iterations(10)->UseRealTime();

static void BM_GetObject_Async_1KB_Concurrent(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::future<GetObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            futures.push_back(client->asyncCall(
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1KB)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async GetObject requests failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 1024);
}
BENCHMARK(BM_GetObject_Async_1KB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->Iterations(5)
    ->UseRealTime();

static void BM_GetObject_Async_1MB_Concurrent(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::future<GetObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            futures.push_back(client->asyncCall(
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1MB)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async GetObject requests failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Async_1MB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->Iterations(5)
    ->UseRealTime();

static void BM_GetObject_Async_4MB_Concurrent(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::future<GetObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            futures.push_back(client->asyncCall(
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey4MB)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async GetObject requests failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 4 * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Async_4MB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->Iterations(5)
    ->UseRealTime();
