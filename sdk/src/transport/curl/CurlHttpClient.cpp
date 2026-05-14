
#include "CurlHttpClient.h"
#include "src/utils/LogUtils.h"

#include <curl/curlver.h>

#include <sstream>

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlHttpClient";

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
static int xferInfoCallback(void* userdata, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    auto* io = static_cast<TransferIO*>(userdata);
    if (io == nullptr) {
        return 0;
    }
    return 0;
}
#else
static int progressCallback(void* userdata, double, double, double, double) {
    auto* io = static_cast<TransferIO*>(userdata);
    if (io == nullptr) {
        return 0;
    }
    return 0;
}
#endif

CurlHttpClient::CurlHttpClient(const HttpTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  kDefaultSyncPoolSize,
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          connOpts_(buildConnectionOptions(options)) {
    (void)CurlGlobalInitializer::instance();
}

CurlHttpClient::CurlHttpClient(const CurlTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  options.poolSize.value_or(kDefaultSyncPoolSize),
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          connOpts_(buildConnectionOptions(options)) {
    (void)CurlGlobalInitializer::instance();
}

ResponseResult CurlHttpClient::send(std::unique_ptr<RequestMessage>& request, RequestContext& context) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter Send", request.get());

    int64_t contentLength = -1;
    curl_slist* list = buildHeaderList(request->headers, request->body, contentLength);

    auto response = std::make_unique<ResponseMessage>();

    CURL* curl = curlContainer_->Acquire();
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) acquire curl handle:%p", request.get(), curl);

    TransferIO io{};
    io.curl = curl;
    io.request = request.get();
    io.response = response.get();
    io.ostreamFactory = &context.ostreamFactory;

    if (request->body != nullptr) {
        io.source = request->body->spanSource();
    }
    io.recvFirstData = true;
    io.recvDataLength = -1;

    curl_easy_setopt(curl, CURLOPT_URL, request->uri.c_str());

    applyHttpMethod(curl, request->method, request->body, contentLength);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &io);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeadersCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &io);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBodyCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &io);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBodyCallback);

    applyConnectionOptions(curl, connOpts_, io.request);

    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferInfoCallback);
#else
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
#endif
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &io);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    } else {
        std::stringstream ss;
        ss << curl_easy_strerror(res) << "." << errbuf;
        if (res == CURLE_WRITE_ERROR) {
            if (io.sink == nullptr) {
                ss << ". Caused by sink is null.";
            } else if (io.sink->bad()) {
                ss << ". Caused by sink is in bad state(Read/writing error on i/o operation).";
            } else if (io.sink->fail()) {
                ss << ". Caused by sink is in fail state(Logical error on i/o operation).";
            }
        } else if (res == CURLE_ABORTED_BY_CALLBACK) {
            // TODO
        }
        context.errorCode = "CURLcode " + std::to_string(res);
        context.errorMessage = ss.str();
    }

    response->statusCode = response_code;
    response->body = io.defaultSink;

    curlContainer_->Release(curl, (res != CURLE_OK));

    curl_slist_free_all(list);

    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, CURLcode:%d, ResponseCode:%d", request.get(), res,
            response_code);

    return response;
}

} // namespace alibabacloud::oss2::transport::curl
