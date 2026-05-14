#include "CurlMultiTransport.h"
#include "src/utils/LogUtils.h"

#include <curl/curlver.h>

#include <sstream>

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlMultiTransport";

struct AsyncTransferContext {
    curl_slist* headers{};

    std::unique_ptr<RequestMessage> request;
    RequestContext context;
    RequestCallback callback;

    std::unique_ptr<ResponseMessage> response;

    TransferIO io;
};

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
                                    RequestContext context,
                                    RequestCallback callback) {
    if (multiHandle_ == nullptr) {
        context.errorCode = "ClientError";
        context.errorMessage = "curl_multi_init failed";
        callback(std::make_error_code(std::errc::operation_not_supported),
                 std::move(request), std::move(context));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        context.errorCode = "ClientError";
        context.errorMessage = "transport is stopped";
        callback(std::make_error_code(std::errc::operation_canceled),
                 std::move(request), std::move(context));
        return;
    }

    CURL* curl = curlContainer_->Acquire();
    if (curl == nullptr) {
        context.errorCode = "ClientError";
        context.errorMessage = "failed to acquire curl handle";
        callback(std::make_error_code(std::errc::resource_unavailable_try_again),
                 std::move(request), std::move(context));
        return;
    }
    if (stopped_.load(std::memory_order_acquire)) {
        curlContainer_->Release(curl, false);
        context.errorCode = "ClientError";
        context.errorMessage = "transport is stopped";
        callback(std::make_error_code(std::errc::operation_canceled),
                 std::move(request), std::move(context));
        return;
    }

    auto ctx = std::make_unique<AsyncTransferContext>();
    ctx->request = std::move(request);
    ctx->context = std::move(context);
    ctx->callback = std::move(callback);
    ctx->response = std::make_unique<ResponseMessage>();

    ctx->io.curl = curl;
    ctx->io.request = ctx->request.get();
    ctx->io.response = ctx->response.get();
    ctx->io.ostreamFactory = &ctx->context.ostreamFactory;
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

void CurlMultiTransport::cleanupTransferContext(AsyncTransferContext* ctx) {
    if (ctx->headers) {
        curl_slist_free_all(ctx->headers);
    }
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
        ctx.release();
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

        long response_code = 0;
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        } else {
            std::stringstream ss;
            ss << curl_easy_strerror(res);
            if (res == CURLE_WRITE_ERROR) {
                if (ctx->io.sink == nullptr) {
                    ss << ". Caused by sink is null.";
                } else if (ctx->io.sink->bad()) {
                    ss << ". Caused by sink is in bad state.";
                } else if (ctx->io.sink->fail()) {
                    ss << ". Caused by sink is in fail state.";
                }
            }
            ctx->context.errorCode = "CURLcode " + std::to_string(res);
            ctx->context.errorMessage = ss.str();
        }

        ctx->response->statusCode = response_code;
        ctx->response->body = ctx->io.defaultSink;

        OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d, ResponseCode:%d",
                res, response_code);

        curl_slist_free_all(ctx->headers);
        ctx->io.curl = nullptr;
        ctx->headers = nullptr;
        curlContainer_->Release(curl, (res != CURLE_OK));

        ctx->callback(std::move(ctx->response),
                      std::move(ctx->request), std::move(ctx->context));
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
        ctx->callback(std::make_error_code(std::errc::operation_canceled),
                      std::move(ctx->request), std::move(ctx->context));
    }

    for (auto* raw : inflightHandles_) {
        std::unique_ptr<AsyncTransferContext> ctx(raw);
        curl_multi_remove_handle(multiHandle_, ctx->io.curl);
        curl_slist_free_all(ctx->headers);
        CURL* curl = ctx->io.curl;
        ctx->io.curl = nullptr;
        ctx->headers = nullptr;
        curlContainer_->Release(curl, false);
        ctx->callback(std::make_error_code(std::errc::operation_canceled),
                      std::move(ctx->request), std::move(ctx->context));
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
