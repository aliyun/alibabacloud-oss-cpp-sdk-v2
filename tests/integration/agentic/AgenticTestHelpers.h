#pragma once

// Shared helpers for the agentic integration tests: client factories, name
// builders, and the prefix-based reaper that bounds the backlog left by the
// two-phase (Disable -> wait ~24h -> Delete) bucket lifecycle. Functions are
// inline so the header can be included from multiple translation units.

#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/AgenticBucketPaginator.h"

#include <memory>
#include <random>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace agentictest {

// The "ab" marker in the prefix is what the reaper filters on.
inline const std::string& bucketNamePrefix() {
    static const std::string prefix = "cpp-sdk-test-ab-";
    return prefix;
}

inline ClientConfiguration testConfig() {
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.accountId = Config::AccountID;
    config.credentialsProvider =
            std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    return config;
}

inline std::shared_ptr<agentic::OSSAgenticBucketClient> makeAgenticClient(bool valid = true) {
    auto config = testConfig();
    if (!valid) {
        config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("invalid-ak", "invalid-sk");
    }
    return std::make_shared<agentic::OSSAgenticBucketClient>(config);
}

inline OSSClient makeBsClient() {
    return agentic::makeBucketSpaceClient(testConfig());
}

// Path-style variants: the resolved full name goes into the request path instead
// of the leftmost host label. Used by the misc path-style scenario.
inline std::shared_ptr<agentic::OSSAgenticBucketClient> makeAgenticClientPathStyle() {
    auto config = testConfig();
    config.usePathStyle = true;
    return std::make_shared<agentic::OSSAgenticBucketClient>(config);
}

inline OSSClient makeBsClientPathStyle() {
    auto config = testConfig();
    config.usePathStyle = true;
    return agentic::makeBucketSpaceClient(config);
}

inline std::string randStr(int n) {
    static const char letters[] = "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 25);
    std::string s;
    s.reserve(n);
    for (int i = 0; i < n; ++i) {
        s.push_back(letters[dist(gen)]);
    }
    return s;
}

inline std::string genBucketName() {
    return bucketNamePrefix() + randStr(6);
}

// buildFullName resolves a short name to the server-side full name
// "{bucket}-{accountId}-{region}-{suffix}" (suffix "ab-apsr" or "bs-apsr").
inline std::string buildFullName(const std::string& bucket, const std::string& suffix) {
    return bucket + "-" + Config::AccountID + "-" + Config::Region + "-" + suffix;
}

// toShortName strips the resolved tail so a listed name can be passed back to a
// client that re-expands short names.
inline std::string toShortName(const std::string& name, const std::string& suffix) {
    const std::string tail = "-" + Config::AccountID + "-" + Config::Region + "-" + suffix;
    if (name.size() >= tail.size() && name.compare(name.size() - tail.size(), tail.size(), tail) == 0) {
        return name.substr(0, name.size() - tail.size());
    }
    return name;
}

// reapBucketSpaces empties and deletes every bucket space of a Disabled agentic
// bucket. Best-effort: all errors are swallowed.
inline void reapBucketSpaces(const std::string& bucket) {
    auto client = makeAgenticClient();
    auto bsClient = makeBsClient();

    auto paginator = agentic::makeAgenticPaginator(client, agentic::models::ListBucketSpacesRequest().setBucket(bucket));
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            break;
        }
        for (const auto& space : outcome.value().getBucketSpaces()) {
            if (!space.name.has_value()) {
                continue;
            }
            auto spaceName = toShortName(space.name.value(), "bs-apsr");

            auto listOutcome = bsClient.listObjectsV2(models::ListObjectsV2Request().setBucket(spaceName));
            if (listOutcome.has_value()) {
                for (const auto& obj : listOutcome.value().getContents()) {
                    (void)bsClient.deleteObject(models::DeleteObjectRequest().setBucket(spaceName).setKey(obj.key));
                }
            }
            (void)bsClient.deleteBucket(models::DeleteBucketRequest().setBucket(spaceName));
        }
    }
}

// reapDisabled deletes leftover buckets from previous runs that carry our prefix
// and are already Disabled (Enabled ones may belong to a concurrent run),
// emptying their bucket spaces first. Best-effort: all errors are swallowed.
inline void reapDisabled() {
    auto client = makeAgenticClient();

    auto paginator = agentic::makeAgenticPaginator(client, agentic::models::ListAgenticBucketsRequest());
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            break;
        }
        for (const auto& summary : outcome.value().getAgenticBuckets()) {
            if (!summary.name.has_value() ||
                summary.name.value().rfind(bucketNamePrefix(), 0) != 0) {
                continue;
            }
            auto bucket = toShortName(summary.name.value(), "ab-apsr");
            // The list summary has no status, so fetch it; only reclaim Disabled.
            auto info = client->getAgenticBucket(agentic::models::GetAgenticBucketRequest().setBucket(bucket));
            if (!info.has_value() || !info.value().hasAgenticBucketInfo() ||
                info.value().getAgenticBucketInfo().status.value_or("") != "Disabled") {
                continue;
            }
            reapBucketSpaces(bucket);
            (void)client->deleteAgenticBucket(agentic::models::DeleteAgenticBucketRequest().setBucket(bucket));
        }
    }
}

// disableAndReap is the shared scenario teardown: disable this run's bucket, then
// reap buckets left disabled by previous runs.
inline void disableAndReap(const std::string& bucket) {
    auto client = makeAgenticClient();
    (void)client->putAgenticBucketStatus(
            agentic::models::PutAgenticBucketStatusRequest().setBucket(bucket).setAgenticBucketStatus(
                    agentic::models::AgenticBucketStatus().setStatus("Disabled")));
    reapDisabled();
}

} // namespace agentictest
} // namespace oss2
} // namespace alibabacloud
