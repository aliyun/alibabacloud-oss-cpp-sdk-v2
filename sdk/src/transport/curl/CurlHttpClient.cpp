
#include "CurlHttpClient.h"
#include "alibabacloud/oss2/Error.h"
#include "src/transport/TransportDefaults.h"
#include "src/utils/LogUtils.h"

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlHttpClient";

std::string CurlHttpClient::getName() const {
    return "curl/" + curlVersionString();
}

CurlHttpClient::CurlHttpClient(const HttpTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  kDefaultMaxConnectionsSync,
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          clientOpts_(buildClientOptions(options)) {
    (void)CurlGlobalInitializer::instance();
}

CurlHttpClient::CurlHttpClient(const CurlTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  options.maxConnections.value_or(kDefaultMaxConnectionsSync),
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          clientOpts_(buildClientOptions(options)) {
    (void)CurlGlobalInitializer::instance();
}

ResponseResult CurlHttpClient::send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter Send", request.get());

    int64_t contentLength = -1;
    curl_slist* list = buildHeaderList(request->headers, request->body, contentLength);

    auto response = std::make_unique<ResponseMessage>();

    CURL* curl = curlContainer_->Acquire();
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) acquire curl handle:%p", request.get(), curl);

    auto ostreamFactory = options.ostreamFactory;

    TransferIO io{};
    io.curl = curl;
    io.request = request.get();
    io.response = response.get();
    io.ostreamFactory = &ostreamFactory;
    if (options.cancellationToken.has_value()) {
        io.cancellationToken = &options.cancellationToken.value();
    }

    if (request->body != nullptr) {
        io.source = request->body->spanSource();
    }
    io.recvFirstData = true;
    io.recvDataLength = -1;

    curl_easy_setopt(curl, CURLOPT_URL, request->uri.c_str());

    applyHttpMethod(curl, request->method, request->body, contentLength);
    applyClientOptions(curl, clientOpts_, io.request);
    applyRequestOptions(curl, &io);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &io);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeadersCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &io);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBodyCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &io);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBodyCallback);

    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    } else {
        curlContainer_->Release(curl, true);
        curl_slist_free_all(list);

        OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, CURLcode:%d", request.get(), res);
        return buildTransportError(res, errbuf, io);
    }

    response->statusCode = response_code;
    response->body = io.defaultSink;

    curlContainer_->Release(curl, false);

    curl_slist_free_all(list);

    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, CURLcode:%d, ResponseCode:%d", request.get(), res,
            response_code);

    return response;
}

} // namespace alibabacloud::oss2::transport::curl
