#pragma once

#include "alibabacloud/oss2/Types.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {


/*
 * The information about the region.
 */
struct ALIBABACLOUD_OSS_API RegionInfo final {
    // The public endpoint of the region.
    std::optional<std::string> internetEndpoint;

    // The internal endpoint of the region.
    std::optional<std::string> internalEndpoint;

    // The acceleration endpoint of the region. The value is always oss-accelerate.aliyuncs.com.
    std::optional<std::string> accelerateEndpoint;

    // The region ID.
    std::optional<std::string> region;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    RegionInfo& setInternetEndpoint(ValueT&& value) {
        internetEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    RegionInfo& setInternalEndpoint(ValueT&& value) {
        internalEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    RegionInfo& setAccelerateEndpoint(ValueT&& value) {
        accelerateEndpoint = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    RegionInfo& setRegion(ValueT&& value) {
        region = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The information about the regions.
 */
struct ALIBABACLOUD_OSS_API RegionInfoList final {
    // The information about the regions.
    std::vector<RegionInfo> regionInfos;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<RegionInfo>>
    RegionInfoList& setRegionInfos(ValueT&& value) {
        regionInfos = std::forward<ValueT>(value);
        return *this;
    }
};


// The request for the DescribeRegions operation.
class ALIBABACLOUD_OSS_API DescribeRegionsRequest final : public RequestModel {
  public:
    DescribeRegionsRequest() = default;

    // The region ID of the request.
    inline const std::string& getRegions() const {
        return getParameterOrEmpty("regions");
    }
    template <typename ValueT = std::string>
    DescribeRegionsRequest& setRegions(ValueT&& value) {
        parameters_.insert_or_assign("regions", std::forward<ValueT>(value));
        return *this;
    }


  private:
};

/// The result for the DescribeRegions operation.
class ALIBABACLOUD_OSS_API DescribeRegionsResult final : public ResultModel {
  public:
    DescribeRegionsResult() = default;
    DescribeRegionsResult(int statusCode, HeaderCollection headers) : ResultModel(statusCode, std::move(headers)) {}


    // The information about the regions.
    inline const RegionInfoList& getRegionInfoList() {
        return body_[0];
    }

    inline bool hasRegionInfoList() const {
        return bodyIsSet_;
    }

    template <typename ValueT = RegionInfoList>
    DescribeRegionsResult& setRegionInfoList(ValueT&& value) {
        bodyIsSet_ = true;
        body_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }


  private:
    std::map<int, RegionInfoList> body_;
    bool bodyIsSet_{};
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
