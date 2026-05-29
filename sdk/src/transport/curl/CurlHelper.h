#pragma once

#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportOptions.h"

#include <curl/curl.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace alibabacloud::oss2::transport::curl {

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
    std::optional<SinkFactory>* sinkFactory{};

    std::unique_ptr<ByteSource> source{};
    ByteWriter* sink{};
    std::shared_ptr<ByteWriter> userSink{};
    std::shared_ptr<std::stringstream> defaultSink{};
    bool recvFirstData{};
    int64_t recvDataLength{};

    const CancellationToken* cancellationToken{};
    const std::function<bool()>* isRequestDisabled{};
};

size_t sendBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
size_t recvBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
size_t recvHeadersCallback(char* buffer, size_t size, size_t nitems, void* userdata);

curl_slist* buildHeaderList(const HeaderCollection& headers, const std::shared_ptr<ByteContent>& body,
                            int64_t& contentLength);

void applyHttpMethod(CURL* curl, const std::string& method, const std::shared_ptr<ByteContent>& body,
                     int64_t contentLength);

struct ClientOptions {
    bool verifySSL{true};
    std::string caPath;
    std::string caFile;
    std::string networkInterface;
    std::string proxyHost;
    unsigned int proxyPort{};
    std::string proxyUserName;
    std::string proxyPassword;
    bool enabledRedirect{false};
    bool enableVerbose{false};
    std::function<void(void*, const RequestMessage*)> requestInterceptor;
    std::function<bool()> isRequestDisabled;
};

void applyClientOptions(CURL* curl, const ClientOptions& opts, const RequestMessage* request);

ClientOptions buildClientOptions(const HttpTransportOptions& options);
ClientOptions buildClientOptions(const CurlTransportOptions& options);

bool headerNameEquals(const std::string& header, const std::string& expect);

void applyRequestOptions(CURL* curl, TransferIO* io);

std::string curlVersionString();

std::error_code make_transport_error_code(int curlCode);

TransportError buildTransportError(CURLcode res, const char* errbuf, const TransferIO& io);

} // namespace alibabacloud::oss2::transport::curl
