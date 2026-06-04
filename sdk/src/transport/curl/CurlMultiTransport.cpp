#include "CurlMultiTransport.h"
#include "alibabacloud/oss2/Error.h"
#include "src/transport/TransportDefaults.h"
#include "src/utils/LogUtils.h"

#include <curl/curlver.h>

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlMultiTransport";

struct AsyncTransferContext {
    curl_slist* headers{};

    std::unique_ptr<RequestMessage> request;
    RequestCallback callback;

    std::unique_ptr<ResponseMessage> response;

    std::optional<SinkFactory> sinkFactory;
    std::optional<CancellationToken> cancellationToken;

    TransferIO io;
    char errbuf[CURL_ERROR_SIZE]{};

    std::unique_ptr<HttpMetrics> metrics;
};

std::string CurlMultiTransport::getName() const {
    return "curl-multi/" + curlVersionString();
}

CurlMultiTransport::CurlMultiTransport(const HttpTransportOptions& options)
    : curlContainer_(std::make_unique<CurlContainer>(kDefaultMaxConnectionsAsync,
                                                     options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                                                     options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
      clientOpts_(buildClientOptions(options)) {
    (void) CurlGlobalInitializer::instance();

    multiHandle_ = curl_multi_init();
    if (multiHandle_ != nullptr) {
        ioThread_ = std::thread([this]() { ioLoop(); });
    }
}

CurlMultiTransport::CurlMultiTransport(const CurlTransportOptions& options)
    : curlContainer_(std::make_unique<CurlContainer>(options.maxConnections.value_or(kDefaultMaxConnectionsAsync),
                                                     options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                                                     options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
      clientOpts_(buildClientOptions(options)) {
    (void) CurlGlobalInitializer::instance();

    multiHandle_ = curl_multi_init();
    if (multiHandle_ != nullptr) {
        ioThread_ = std::thread([this]() { ioLoop(); });
    }
}

CurlMultiTransport::~CurlMultiTransport() {
    stopped_.store(true, std::memory_order_release);

#if LIBCURL_VERSION_NUM >= 0x074400 // 7.68.0
    curl_multi_wakeup(multiHandle_);
#endif

    if (ioThread_.joinable()) {
        ioThread_.join();
    }

    if (multiHandle_) {
        curl_multi_cleanup(multiHandle_);
    }
}

void CurlMultiTransport::sendAsync(std::unique_ptr<RequestMessage> request, const RequestOptions& options,
                                   RequestCallback callback) {
    if (multiHandle_ == nullptr) {
        callback(TransportError{std::make_error_code(std::errc::operation_not_supported), "ClientError",
                                "curl_multi_init failed"},
                 std::move(request));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        callback(
            TransportError{std::make_error_code(std::errc::operation_canceled), "ClientError", "transport is stopped"},
            std::move(request));
        return;
    }

    CURL* curl = curlContainer_->Acquire();

    if (curl == nullptr) {
        callback(TransportError{std::make_error_code(std::errc::resource_unavailable_try_again), "ClientError",
                                "failed to acquire curl handle"},
                 std::move(request));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        curlContainer_->Release(curl, false);
        callback(
            TransportError{std::make_error_code(std::errc::operation_canceled), "ClientError", "transport is stopped"},
            std::move(request));
        return;
    }

    auto ctx = std::make_unique<AsyncTransferContext>();
    ctx->request = std::move(request);
    ctx->callback = std::move(callback);
    ctx->response = std::make_unique<ResponseMessage>();
    ctx->sinkFactory = options.sinkFactory;
    ctx->cancellationToken = options.cancellationToken;

    ctx->io.curl = curl;
    ctx->io.request = ctx->request.get();
    ctx->io.response = ctx->response.get();
    ctx->io.sinkFactory = &ctx->sinkFactory;
    if (ctx->cancellationToken.has_value()) {
        ctx->io.cancellationToken = &ctx->cancellationToken.value();
    }
    if (ctx->request->body != nullptr) {
        ctx->io.source = ctx->request->body->spanSource();
    }
    ctx->io.recvFirstData = true;
    ctx->io.recvDataLength = -1;

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pendingRequests_.push_back(std::move(ctx));
    }

#if LIBCURL_VERSION_NUM >= 0x074400 // 7.68.0
    curl_multi_wakeup(multiHandle_);
#endif
}

void CurlMultiTransport::setupCurlHandle(AsyncTransferContext* ctx) {
    CURL* curl = ctx->io.curl;

    ctx->metrics = makeHttpMetrics(clientOpts_.collectMetrics);
    beforeRequestMetrics(ctx->metrics.get());

    int64_t contentLength = -1;
    curl_slist* list = buildHeaderList(ctx->request->headers, ctx->request->body, contentLength);
    ctx->headers = list;

    curl_easy_setopt(curl, CURLOPT_URL, ctx->request->uri.c_str());

    applyHttpMethod(curl, ctx->request->method, ctx->request->body, contentLength);
    applyClientOptions(curl, clientOpts_, ctx->io.request);
    applyRequestOptions(curl, &ctx->io);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeadersCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBodyCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBodyCallback);

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, ctx->errbuf);
    ctx->errbuf[0] = 0;

    curl_easy_setopt(curl, CURLOPT_PRIVATE, ctx);
}

void CurlMultiTransport::drainPending() {
    std::vector<std::unique_ptr<AsyncTransferContext>> batch;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        batch.swap(pendingRequests_);
    }

    for (auto& ctx : batch) {
        auto* raw = ctx.get();
        setupCurlHandle(raw);
        curl_multi_add_handle(multiHandle_, raw->io.curl);
        inflightHandles_.insert(raw);
        (void) ctx.release();
    }
}

void CurlMultiTransport::processCompleted() {
    CURLMsg* msg;
    int msgs_left = 0;

    while ((msg = curl_multi_info_read(multiHandle_, &msgs_left)) != nullptr) {
        if (msg->msg != CURLMSG_DONE) {
            continue;
        }

        CURL* curl = msg->easy_handle;
        CURLcode res = msg->data.result;

        AsyncTransferContext* raw = nullptr;
        curl_easy_getinfo(curl, CURLINFO_PRIVATE, &raw);
        inflightHandles_.erase(raw);
        std::unique_ptr<AsyncTransferContext> ctx(raw);
        curl_multi_remove_handle(multiHandle_, curl);

        curl_slist_free_all(ctx->headers);
        ctx->io.curl = nullptr;
        ctx->headers = nullptr;

        afterRequestMetrics(ctx->metrics.get(), curl);

        if (res == CURLE_OK) {
            long response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            ctx->response->statusCode = response_code;
            ctx->response->body = ctx->io.defaultSink;
            ctx->response->metrics = std::move(ctx->metrics);

            OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d, ResponseCode:%d", res,
                    response_code);

            curlContainer_->Release(curl, false);
            ctx->callback(std::move(ctx->response), std::move(ctx->request));
        } else {
            OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d", res);

            curlContainer_->Release(curl, true);

            ctx->callback(buildTransportError(res, ctx->errbuf, ctx->io), std::move(ctx->request));
        }
    }
}

void CurlMultiTransport::cleanupInflight() {
    std::vector<std::unique_ptr<AsyncTransferContext>> pending;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pending.swap(pendingRequests_);
    }
    for (auto& ctx : pending) {
        curlContainer_->Release(ctx->io.curl, false);
        ctx->io.curl = nullptr;
        ctx->callback(
            TransportError{std::make_error_code(std::errc::operation_canceled), "ClientError", "transport is stopped"},
            std::move(ctx->request));
    }

    for (auto* raw : inflightHandles_) {
        std::unique_ptr<AsyncTransferContext> ctx(raw);
        curl_multi_remove_handle(multiHandle_, ctx->io.curl);
        curl_slist_free_all(ctx->headers);
        CURL* curl = ctx->io.curl;
        ctx->io.curl = nullptr;
        ctx->headers = nullptr;
        curlContainer_->Release(curl, false);
        ctx->callback(
            TransportError{std::make_error_code(std::errc::operation_canceled), "ClientError", "transport is stopped"},
            std::move(ctx->request));
    }
    inflightHandles_.clear();
}

void CurlMultiTransport::ioLoop() {
    OSS_LOG(LogLevel::LogInfo, TAG, "IO loop started");

    while (!stopped_.load(std::memory_order_acquire)) {
        drainPending();

        int still_running = 0;
        curl_multi_perform(multiHandle_, &still_running);

        processCompleted();

        if (stopped_.load(std::memory_order_acquire)) {
            break;
        }

        int numfds = 0;
#if LIBCURL_VERSION_NUM >= 0x074200 // 7.66.0
        curl_multi_poll(multiHandle_, nullptr, 0, 200, &numfds);
#else
        curl_multi_wait(multiHandle_, nullptr, 0, 200, &numfds);
#endif
    }

    int still_running = 0;
    curl_multi_perform(multiHandle_, &still_running);
    processCompleted();

    cleanupInflight();

    OSS_LOG(LogLevel::LogInfo, TAG, "IO loop stopped");
}

} // namespace alibabacloud::oss2::transport::curl
