#include "PerfConfig.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <deque>
#include <future>
#include <string>
#include <vector>

using namespace alibabacloud::oss2;

namespace {

class NullStreamBuf : public std::streambuf {
  protected:
    std::streamsize xsputn(const char*, std::streamsize n) override { return n; }
    int overflow(int c) override { return c; }
};

OStreamFactory makeDiscardFactory() {
    OStreamFactory factory;
    factory.supplier = [](std::int64_t) -> std::shared_ptr<std::ostream> {
        static thread_local auto buf = std::make_shared<NullStreamBuf>();
        return std::make_shared<std::ostream>(buf.get());
    };
    factory.isOneShot = false;
    return factory;
}

} // namespace

static const std::string kGetKey1KB = std::string(perf::kKeyPrefix) + "perf-get-fixture-1k.dat";
static const std::string kGetKey1MB = std::string(perf::kKeyPrefix) + "perf-get-fixture-1m.dat";
static const std::string kGetKey4MB = std::string(perf::kKeyPrefix) + "perf-get-fixture-4m.dat";

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
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1KB).setOStreamFactory(makeDiscardFactory()));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_GetObject_Sync_1KB)->UseRealTime();

static void BM_GetObject_Sync_1MB(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1MB).setOStreamFactory(makeDiscardFactory()));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Sync_1MB)->UseRealTime();

static void BM_GetObject_Sync_4MB(benchmark::State& state) {
    ensureFixtureObjects();
    auto client = perf::GetSyncClient();
    auto& cfg = perf::GetConfig();

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey4MB).setOStreamFactory(makeDiscardFactory()));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 4 * 1024 * 1024);
}
BENCHMARK(BM_GetObject_Sync_4MB)->UseRealTime();

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
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1KB).setOStreamFactory(makeDiscardFactory())));
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
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey1MB).setOStreamFactory(makeDiscardFactory())));
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
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(kGetKey4MB).setOStreamFactory(makeDiscardFactory())));
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
    ->UseRealTime();

static std::string ensureCustomFixture() {
    static std::string key;
    if (!key.empty()) return key;

    auto& cfg = perf::GetConfig();
    key = std::string(perf::kKeyPrefix) + "perf-get-fixture-custom.dat";
    std::string data(cfg.objectSize, 'U');
    auto body = RequestBody::FromMemory(data.data(), data.size());
    perf::GetSyncClient()->putObject(
        models::PutObjectRequest().setBucket(cfg.bucket).setKey(key).setBody(body));
    return key;
}

static void BM_GetObject_Async_Custom_Concurrent(benchmark::State& state) {
    auto& cfg = perf::GetConfig();
    if (cfg.concurrency <= 0) {
        state.SkipWithError("Use --concurrency <N> to run this test");
        return;
    }
    auto customKey = ensureCustomFixture();
    auto client = perf::GetAsyncClient();
    const int concurrency = cfg.concurrency;
    const int objectSize = cfg.objectSize;

    for (auto _ : state) {
        std::vector<std::future<GetObjectOutcome>> futures;
        futures.reserve(concurrency);
        for (int i = 0; i < concurrency; i++) {
            futures.push_back(client->asyncCall(
                models::GetObjectRequest().setBucket(cfg.bucket).setKey(customKey).setOStreamFactory(makeDiscardFactory())));
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
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * concurrency * objectSize);
}
BENCHMARK(BM_GetObject_Async_Custom_Concurrent)->UseRealTime();

static void BM_GetObject_Async_Sustained(benchmark::State& state) {
    auto customKey = ensureCustomFixture();
    auto& cfg = perf::GetConfig();
    auto client = perf::GetAsyncClient();
    const int objectSize = cfg.objectSize;
    int64_t total = 0;
    std::deque<std::future<GetObjectOutcome>> inflight;

    for (auto _ : state) {
        inflight.push_back(client->asyncCall(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(customKey).setOStreamFactory(makeDiscardFactory())));
        total++;

        while (!inflight.empty() &&
               inflight.front().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto outcome = inflight.front().get();
            inflight.pop_front();
            if (!outcome.has_value()) {
                state.SkipWithError("GetObject failed");
                break;
            }
        }
    }

    for (auto& f : inflight) {
        (void)f.get();
    }

    state.SetItemsProcessed(total);
    state.SetBytesProcessed(static_cast<int64_t>(total) * objectSize);
}
BENCHMARK(BM_GetObject_Async_Sustained)->UseRealTime();

static void BM_GetObject_Sync_Sustained(benchmark::State& state) {
    auto customKey = ensureCustomFixture();
    auto& cfg = perf::GetConfig();
    auto client = perf::GetSyncClient();
    const int objectSize = cfg.objectSize;
    int64_t total = 0;

    for (auto _ : state) {
        auto outcome = client->getObject(
            models::GetObjectRequest().setBucket(cfg.bucket).setKey(customKey).setOStreamFactory(makeDiscardFactory()));
        if (!outcome.has_value()) {
            state.SkipWithError("GetObject failed");
            break;
        }
        total++;
    }

    state.SetItemsProcessed(total);
    state.SetBytesProcessed(static_cast<int64_t>(total) * objectSize);
}
BENCHMARK(BM_GetObject_Sync_Sustained)->UseRealTime();
