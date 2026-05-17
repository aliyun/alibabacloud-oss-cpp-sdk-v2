#include "CurlMultiTransport.h"
#include "src/utils/LogUtils.h"

#include <curl/curlver.h>

#include <sstream>

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlMultiTransport";

struct AsyncTransferContext {
    curl_slist* headers{};

    std::unique_ptr<RequestMessage> request;
    RequestCallback callback;

    std::unique_ptr<ResponseMessage> response;

    std::optional<OStreamFactory> ostreamFactory;

    TransferIO io;
    char errbuf[CURL_ERROR_SIZE]{};
};

std::string CurlMultiTransport::getName() const {
    return "curl-multi/" + curlVersionString();
}

CurlMultiTransport::CurlMultiTransport(const HttpTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  kDefaultAsyncPoolSize,
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          connOpts_(buildConnectionOptions(options)) {
    (void)CurlGlobalInitializer::instance();

    multiHandle_ = curl_multi_init();
    if (multiHandle_ != nullptr) {
        ioThread_ = std::thread([this]() { ioLoop(); });
    }
}

CurlMultiTransport::CurlMultiTransport(const CurlTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  options.poolSize.value_or(kDefaultAsyncPoolSize),
                  options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs),
                  options.connectTimeout.value_or(kDefaultConnectTimeoutMs))),
          connOpts_(buildConnectionOptions(options)) {
    (void)CurlGlobalInitializer::instance();

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

void CurlMultiTransport::sendAsync(std::unique_ptr<RequestMessage> request,
                                    const RequestOptions& options,
                                    RequestCallback callback) {
    if (multiHandle_ == nullptr) {
        callback(TransportError{std::make_error_code(std::errc::operation_not_supported),
                 "ClientError", "curl_multi_init failed"},
                 std::move(request));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        callback(TransportError{std::make_error_code(std::errc::operation_canceled),
                 "ClientError", "transport is stopped"},
                 std::move(request));
        return;
    }

    CURL* curl = curlContainer_->Acquire();
    if (curl == nullptr) {
        callback(TransportError{std::make_error_code(std::errc::resource_unavailable_try_again),
                 "ClientError", "failed to acquire curl handle"},
                 std::move(request));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        curlContainer_->Release(curl, false);
        callback(TransportError{std::make_error_code(std::errc::operation_canceled),
                 "ClientError", "transport is stopped"},
                 std::move(request));
        return;
    }

    auto ctx = std::make_unique<AsyncTransferContext>();
    ctx->request = std::move(request);
    ctx->callback = std::move(callback);
    ctx->response = std::make_unique<ResponseMessage>();
    ctx->ostreamFactory = options.ostreamFactory;

    ctx->io.curl = curl;
    ctx->io.request = ctx->request.get();
    ctx->io.response = ctx->response.get();
    ctx->io.ostreamFactory = &ctx->ostreamFactory;
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

    int64_t contentLength = -1;
    curl_slist* list = buildHeaderList(ctx->request->headers, ctx->request->body, contentLength);
    ctx->headers = list;

    curl_easy_setopt(curl, CURLOPT_URL, ctx->request->uri.c_str());

    applyHttpMethod(curl, ctx->request->method, ctx->request->body, contentLength);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeadersCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBodyCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ctx->io);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBodyCallback);

    applyConnectionOptions(curl, connOpts_, ctx->io.request);

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
        (void)ctx.release();
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

        if (res == CURLE_OK) {
            long response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            ctx->response->statusCode = response_code;
            ctx->response->body = ctx->io.defaultSink;

            OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d, ResponseCode:%d",
                    res, response_code);

            curlContainer_->Release(curl, false);
            ctx->callback(std::move(ctx->response), std::move(ctx->request));
        } else {
            std::stringstream ss;
            ss << curl_easy_strerror(res) << "." << ctx->errbuf;
            if (res == CURLE_WRITE_ERROR) {
                if (ctx->io.sink == nullptr) {
                    ss << ". Caused by sink is null.";
                } else if (ctx->io.sink->bad()) {
                    ss << ". Caused by sink is in bad state.";
                } else if (ctx->io.sink->fail()) {
                    ss << ". Caused by sink is in fail state.";
                }
            }

            OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d", res);

            curlContainer_->Release(curl, true);

            ctx->callback(TransportError{make_transport_error_code(res),
                                         "CURLcode " + std::to_string(res), ss.str()},
                          std::move(ctx->request));
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
        ctx->callback(TransportError{std::make_error_code(std::errc::operation_canceled),
                      "ClientError", "transport is stopped"},
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
        ctx->callback(TransportError{std::make_error_code(std::errc::operation_canceled),
                      "ClientError", "transport is stopped"},
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
