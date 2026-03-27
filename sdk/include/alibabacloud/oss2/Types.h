
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>


namespace alibabacloud {
namespace oss2 {

enum class AddressStyleType { VirtualHosted, Path, CName };

enum class LogLevel {
    LogOff = 0,
    LogFatal,
    LogError,
    LogWarn,
    LogInfo,
    LogDebug,
    LogTrace,
    LogAll,
};

enum class FeatureFlagsType {
    /**
     * If the client time is different from server time by more than about 15 minutes,
     * the requests your application makes will be signed with the incorrect time, and the server will reject them.
     * The feature to help to identify this case, and SDK will correct for clock skew.
     */
    CorrectClockSkew = (1 << 0),

    /**
     * Content-Type is automatically added based on the object name if not specified.
     * This feature takes effect for PutObject, AppendObject and InitiateMultipartUpload
     */
    AutoDetectMimeType = (1 << 1),

    /**
     * Check data integrity of uploads via the crc64.
     * This feature takes effect for PutObject, AppendObject, UploadPart, Uploader.UploadFrom and
     * Uploader.UploadFile
     */
    EnableCRC64CheckUpload = (1 << 2),

    /**
     * Check data integrity of downloads via the crc64.
     * This feature takes effect for Downloader.DownloadFile
     */
    EnableCRC64CheckDonwload = (1 << 3),
};

struct ALIBABACLOUD_OSS_API caseSensitiveLess {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs < rhs;
    } // namespace oss2
};    // namespace alibabacloud

struct ALIBABACLOUD_OSS_API caseInsensitiveLess {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        auto first1 = lhs.begin(), last1 = lhs.end();
        auto first2 = rhs.begin(), last2 = rhs.end();
        while (first1 != last1) {
            if (first2 == last2)
                return false;
            auto first1_ch = ::tolower(*first1);
            auto first2_ch = ::tolower(*first2);
            if (first1_ch != first2_ch) {
                return (first1_ch < first2_ch);
            }
            ++first1;
            ++first2;
        }
        return (first2 != last2);
    }
};

struct ALIBABACLOUD_OSS_API ProgressCallback {
    void operator()(std::size_t increment, std::size_t transferred, std::int64_t total) const {
        if (callback) {
            callback(increment, transferred, total, userdata);
        }
    }
    std::function<void(std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t userdata)>
            callback;
    std::uintptr_t userdata{};
};

struct ALIBABACLOUD_OSS_API OStreamFactory {
    /*
     * size: the max length of data to write, -1 means the length is unknown
     */
    std::shared_ptr<std::ostream> operator()(std::int64_t size) const {
        return supplier ? supplier(size) : nullptr;
    }
    std::function<std::shared_ptr<std::ostream>(std::int64_t size)> supplier;
    bool isOneShot{};
};

typedef void (*LogCallback)(LogLevel level, const std::string& stream);
using AttributeValue = std::variant<bool, std::vector<std::string>, std::int64_t>;
using AttributeMap = std::map<std::string, AttributeValue>;
using MetaData = std::map<std::string, std::string, caseInsensitiveLess>;
using HeaderCollection = std::map<std::string, std::string, caseInsensitiveLess>;
using ParameterCollection = std::map<std::string, std::string, caseSensitiveLess>;


class ALIBABACLOUD_OSS_API RequestModel {
  public:
    RequestModel() = default;
    RequestModel(HeaderCollection headers, ParameterCollection parameters)
            : headers_(std::move(headers)), parameters_(std::move(parameters)) {}

    virtual ~RequestModel() = default;

    inline const HeaderCollection& getHeaders() const {
        return headers_;
    }

    inline const ParameterCollection& getParameters() const {
        return parameters_;
    }

    inline void addHeader(const std::string& key, const std::string& value) {
        headers_.insert_or_assign(key, value);
    }

    inline void addParameter(const std::string& key, const std::string& value) {
        parameters_.insert_or_assign(key, value);
    }

  protected:
    const std::string& getHeaderOrEmpty(const std::string& key) const;
    const std::string& getParameterOrEmpty(const std::string& key) const;

    std::int32_t getHeaderAsInt32Or(const std::string& key, std::int32_t value = -1) const;
    std::int64_t getHeaderAsInt64Or(const std::string& key, std::int64_t value = -1LL) const;
    bool getHeaderAsBoolOr(const std::string& key, bool value = false) const;

    std::int32_t getParameterAsInt32Or(const std::string& key, std::int32_t value = -1) const;
    std::int64_t getParameterAsInt64Or(const std::string& key, std::int64_t value = -1LL) const;
    bool getParameterAsBoolOr(const std::string& key, bool value = false) const;

    HeaderCollection headers_;
    ParameterCollection parameters_;
};

class ALIBABACLOUD_OSS_API ResultModel {
  public:
    ResultModel() = default;
    ResultModel(int statusCode, HeaderCollection headers)
            : status_(""), statusCode_(statusCode), headers_(std::move(headers)) {}
    virtual ~ResultModel() = default;

    inline const HeaderCollection& getHeaders() const {
        return headers_;
    }

    inline int getStatusCode() const {
        return statusCode_;
    }

    inline const std::string& getRequestId() const {
        return getHeaderOrEmpty("x-oss-request-id");
    }

  protected:
    std::string status_;
    int statusCode_{};
    HeaderCollection headers_;
    const std::string& getHeaderOrEmpty(const std::string& key) const;
    std::int64_t getHeaderAsInt64Or(const std::string& key, std::int64_t value = -1LL) const;
};
} // namespace oss2
} // namespace alibabacloud