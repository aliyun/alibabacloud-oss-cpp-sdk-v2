#include "PerfConfig.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <deque>
#include <future>
#include <string>
#include <vector>

using namespace alibabacloud::oss2;

static std::string makeKey(const char* prefix, int64_t iter) {
    return std::string(perf::kKeyPrefix) + prefix + std::to_string(iter) + ".dat";
}

static void BM_PutObject_Sync_1KB(benchmark::State& state) {
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();
    std::string payload(1024, 'A');
    int64_t i = 0;
    for (auto _ : state) {
        auto key = makeKey("perf-put-sync-1k-", i++);
        auto body = RequestBody::fromMemory(payload.data(), payload.size());
        auto outcome = client->putObject(
            models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body));
        if (!outcome.has_value()) {
            state.SkipWithError("PutObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_PutObject_Sync_1KB)->UseRealTime();

static void BM_PutObject_Sync_1MB(benchmark::State& state) {
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();
    std::string payload(1024 * 1024, 'B');
    int64_t i = 0;
    for (auto _ : state) {
        auto key = makeKey("perf-put-sync-1m-", i++);
        auto body = RequestBody::fromMemory(payload.data(), payload.size());
        auto outcome = client->putObject(
            models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body));
        if (!outcome.has_value()) {
            state.SkipWithError("PutObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_PutObject_Sync_1MB)->UseRealTime();

static void BM_PutObject_Sync_4MB(benchmark::State& state) {
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();
    std::string payload(4 * 1024 * 1024, 'E');
    int64_t i = 0;
    for (auto _ : state) {
        auto key = makeKey("perf-put-sync-4m-", i++);
        auto body = RequestBody::fromMemory(payload.data(), payload.size());
        auto outcome = client->putObject(
            models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body));
        if (!outcome.has_value()) {
            state.SkipWithError("PutObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 4 * 1024 * 1024);
}
BENCHMARK(BM_PutObject_Sync_4MB)->UseRealTime();

static void BM_PutObject_Async_1KB_Concurrent(benchmark::State& state) {
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));
    std::string payload(1024, 'C');
    int64_t batch = 0;

    for (auto _ : state) {
        std::vector<std::future<PutObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            auto key = makeKey("perf-put-async-1k-", batch * concurrency + i);
            auto body = RequestBody::fromMemory(payload.data(), payload.size());
            futures.push_back(client->asyncCall(
                models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async PutObject requests failed");
            break;
        }
        batch++;
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 1024);
}
BENCHMARK(BM_PutObject_Async_1KB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->UseRealTime();

static void BM_PutObject_Async_1MB_Concurrent(benchmark::State& state) {
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));
    std::string payload(1024 * 1024, 'D');
    int64_t batch = 0;

    for (auto _ : state) {
        std::vector<std::future<PutObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            auto key = makeKey("perf-put-async-1m-", batch * concurrency + i);
            auto body = RequestBody::fromMemory(payload.data(), payload.size());
            futures.push_back(client->asyncCall(
                models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async PutObject requests failed");
            break;
        }
        batch++;
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 1024 * 1024);
}
BENCHMARK(BM_PutObject_Async_1MB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->UseRealTime();

static void BM_PutObject_Async_4MB_Concurrent(benchmark::State& state) {
    auto client = perf::GetAsyncClient();
    auto& cfg = perf::GetConfig();
    const int concurrency = static_cast<int>(state.range(0));
    std::string payload(4 * 1024 * 1024, 'F');
    int64_t batch = 0;

    for (auto _ : state) {
        std::vector<std::future<PutObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            auto key = makeKey("perf-put-async-4m-", batch * concurrency + i);
            auto body = RequestBody::fromMemory(payload.data(), payload.size());
            futures.push_back(client->asyncCall(
                models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async PutObject requests failed");
            break;
        }
        batch++;
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(state.iterations() * concurrency * 4 * 1024 * 1024);
}
BENCHMARK(BM_PutObject_Async_4MB_Concurrent)
    ->Arg(10)->Arg(50)->Arg(100)->Arg(200)
    ->UseRealTime();

static void BM_PutObject_Async_Custom_Concurrent(benchmark::State& state) {
    auto& cfg = perf::GetConfig();
    if (cfg.concurrency <= 0) {
        state.SkipWithError("Use --concurrency <N> to run this test");
        return;
    }
    auto client = perf::GetAsyncClient();
    const int concurrency = cfg.concurrency;
    const int objectSize = cfg.objectSize;
    std::string payload(objectSize, 'G');
    int64_t batch = 0;

    for (auto _ : state) {
        std::vector<std::future<PutObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            auto key = makeKey("perf-put-custom-", batch * concurrency + i);
            auto body = RequestBody::fromMemory(payload.data(), payload.size());
            futures.push_back(client->asyncCall(
                models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body)));
        }
        int failures = 0;
        for (auto& f : futures) {
            auto outcome = f.get();
            if (!outcome.has_value()) failures++;
        }
        if (failures > 0) {
            state.SkipWithError("Some async PutObject requests failed");
            break;
        }
        batch++;
    }
    state.SetItemsProcessed(state.iterations() * concurrency);
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * concurrency * objectSize);
}
BENCHMARK(BM_PutObject_Async_Custom_Concurrent)->UseRealTime();

static void BM_PutObject_Async_Sustained(benchmark::State& state) {
    auto& cfg = perf::GetConfig();
    auto client = perf::GetAsyncClient();
    std::string payload(cfg.objectSize, 'H');
    int64_t total = 0;
    std::deque<std::future<PutObjectOutcome>> inflight;

    for (auto _ : state) {
        auto key = makeKey("perf-put-sustained-", total++);
        auto body = RequestBody::fromMemory(payload.data(), payload.size());
        inflight.push_back(client->asyncCall(
            models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body)));

        while (!inflight.empty() &&
               inflight.front().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto outcome = inflight.front().get();
            inflight.pop_front();
            if (!outcome.has_value()) {
                state.SkipWithError("PutObject failed");
                break;
            }
        }
    }

    for (auto& f : inflight) {
        (void)f.get();
    }

    state.SetItemsProcessed(total);
    state.SetBytesProcessed(static_cast<int64_t>(total) * cfg.objectSize);
}
BENCHMARK(BM_PutObject_Async_Sustained)->UseRealTime();

static void BM_PutObject_Sync_Sustained(benchmark::State& state) {
    auto& cfg = perf::GetConfig();
    auto client = perf::GetSyncClient();
    std::string payload(cfg.objectSize, 'I');
    int64_t total = 0;

    for (auto _ : state) {
        auto key = makeKey("perf-put-sync-sustained-", total++);
        auto body = RequestBody::fromMemory(payload.data(), payload.size());
        auto outcome = client->putObject(
            models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body));
        if (!outcome.has_value()) {
            state.SkipWithError("PutObject failed");
            break;
        }
    }

    state.SetItemsProcessed(total);
    state.SetBytesProcessed(static_cast<int64_t>(total) * cfg.objectSize);
}
BENCHMARK(BM_PutObject_Sync_Sustained)->UseRealTime();
