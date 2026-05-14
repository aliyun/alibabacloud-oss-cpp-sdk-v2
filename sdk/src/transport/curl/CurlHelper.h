#pragma once

#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportOptions.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <curl/curl.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace alibabacloud::oss2::transport::curl {

// Transport defaults, aligned with alibabacloud-oss-go-sdk-v2
constexpr long kDefaultConnectTimeoutMs = 5000;        // 5s
constexpr long kDefaultReadWriteTimeoutMs = 10000;     // 10s
constexpr unsigned int kDefaultSyncPoolSize = 16;
constexpr unsigned int kDefaultAsyncPoolSize = 100;

class CurlGlobalInitializer {
  public:
    static CurlGlobalInitializer& instance() {
        static CurlGlobalInitializer inst;
        return inst;
    }

    CurlGlobalInitializer(const CurlGlobalInitializer&) = delete;
    CurlGlobalInitializer& operator=(const CurlGlobalInitializer&) = delete;

  private:
    CurlGlobalInitializer() {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlGlobalInitializer() {
        curl_global_cleanup();
    }
};

struct TransferIO {
    CURL* curl{};
    RequestMessage* request{};
    ResponseMessage* response{};
    std::optional<OStreamFactory>* ostreamFactory{};

    std::unique_ptr<ByteSource> source{};
    std::ostream* sink{};
    std::shared_ptr<std::ostream> userSink{};
    std::shared_ptr<std::stringstream> defaultSink{};
    bool recvFirstData{};
    int64_t recvDataLength{};
};

size_t sendBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
size_t recvBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
size_t recvHeadersCallback(char* buffer, size_t size, size_t nitems, void* userdata);

curl_slist* buildHeaderList(const HeaderCollection& headers,
                            const std::shared_ptr<ByteContent>& body,
                            int64_t& contentLength);

void applyHttpMethod(CURL* curl, const std::string& method,
                     const std::shared_ptr<ByteContent>& body,
                     int64_t contentLength);

struct ConnectionOptions {
    bool verifySSL{true};
    std::string caPath;
    std::string caFile;
    std::string networkInterface;
    std::string proxyHost;
    unsigned int proxyPort{};
    std::string proxyUserName;
    std::string proxyPassword;
    bool enableVerbose{false};
    std::function<void(void*, const RequestMessage*)> requestInterceptor;
};

void applyConnectionOptions(CURL* curl, const ConnectionOptions& opts,
                            const RequestMessage* request);

ConnectionOptions buildConnectionOptions(const HttpTransportOptions& options);
ConnectionOptions buildConnectionOptions(const CurlTransportOptions& options);

bool headerNameEquals(const std::string& header, const std::string& expect);

} // namespace alibabacloud::oss2::transport::curl
