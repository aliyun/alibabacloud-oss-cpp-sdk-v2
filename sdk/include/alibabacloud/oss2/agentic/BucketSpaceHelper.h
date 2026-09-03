#pragma once

#include "alibabacloud/oss2/ClientConfiguration.h"

#include <string>

namespace alibabacloud {
namespace oss2 {
namespace agentic {

/**
 * @brief Helper that resolves a bucket space prefix into a physical bucket name.
 *
 * The physical bucket name follows the pattern:
 *   {prefix}-{accountId}-{region}-bs-apsr
 */
class BucketSpaceHelper {
  public:
    explicit BucketSpaceHelper(const ClientConfiguration& config)
        : accountId_(config.accountId.value_or("")), region_(config.region.value_or("")) {}

    BucketSpaceHelper(std::string accountId, std::string region)
        : accountId_(std::move(accountId)), region_(std::move(region)) {}

    inline std::string toBucketName(const std::string& prefix) const {
        return prefix + "-" + accountId_ + "-" + region_ + "-bs-apsr";
    }

    inline const std::string& getAccountId() const {
        return accountId_;
    }

    inline const std::string& getRegion() const {
        return region_;
    }

  private:
    std::string accountId_;
    std::string region_;
};

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
