#include "CurlMultiTransport.h"
#include "src/utils/LogUtils.h"

#include <curl/curlver.h>

#include <charconv>
#include <sstream>

namespace alibabacloud::oss2::transport::curl {

static const char* TAG = "CurlMultiTransport";

struct AsyncTransferContext {
    CURL* curl{};
    curl_slist* headers{};

    std::unique_ptr<RequestMessage> request;
    RequestContext context;
    RequestCallback callback;

    std::unique_ptr<ResponseMessage> response;

    std::unique_ptr<ByteSource> source{};

    std::ostream* sink{};
    std::shared_ptr<std::ostream> userSink{};
    std::shared_ptr<std::stringstream> defaultSink{};
    bool recvFirstData{};
    int64_t recvDataLength{};
};

static size_t sendBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<AsyncTransferContext*>(userdata);
    if (ctx == nullptr || ctx->request == nullptr) {
        return 0;
    }

    size_t wanted = size * nmemb;
    size_t got = 0;
    if (ctx->source != nullptr) {
        got = ctx->source->readToCount(reinterpret_cast<uint8_t*>(ptr), wanted);
        if (got < wanted) {
            auto st = ctx->source->state();
            if (st != 0 && (st & std::ios::eofbit) == 0) {
                return CURL_READFUNC_ABORT;
            }
        }
    }
    return got;
}

static size_t recvBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<AsyncTransferContext*>(userdata);
    const size_t wanted = size * nmemb;

    if (ctx == nullptr || ctx->response == nullptr) {
        return 0;
    }

    if (ctx->recvFirstData) {
        long response_code = 0;
        curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code / 100 != 2 || response_code == 203 || !ctx->context.ostreamFactory) {
            ctx->defaultSink = std::make_shared<std::stringstream>();
            ctx->sink = ctx->defaultSink.get();
        } else {
            ctx->userSink = ctx->context.ostreamFactory.value()(ctx->recvDataLength);
            ctx->sink = ctx->userSink.get();
        }
        ctx->recvFirstData = false;
    }

    if (ctx->sink == nullptr || ctx->sink->fail()) {
        return 0;
    }

    ctx->sink->write(ptr, static_cast<std::streamsize>(wanted));
    if (ctx->sink->bad()) {
        return 0;
    }
    return wanted;
}

static size_t recvHeaders(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* ctx = static_cast<AsyncTransferContext*>(userdata);
    const size_t wanted = nitems * size;

    std::string line(buffer);
    auto pos = line.find(':');
    if (pos != line.npos) {
        size_t posEnd = line.rfind('\r');
        if (posEnd != line.npos) {
            posEnd = posEnd - pos - 2;
        }
        auto name = line.substr(0, pos);
        auto value = line.substr(pos + 2, posEnd);
        ctx->response->headers.emplace(std::move(name), std::move(value));
    }

    if (wanted == 2 && (buffer[0] == 0x0D) && (buffer[1] == 0x0A)) {
        if (ctx->response->headers.find("Content-Length") != ctx->response->headers.end()) {
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 55, 0)
            curl_off_t dval;
            curl_easy_getinfo(ctx->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dval);
            ctx->recvDataLength = (int64_t) dval;
#else
            double dval;
            curl_easy_getinfo(ctx->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
            ctx->recvDataLength = (int64_t) dval;
#endif
        }
    }
    return wanted;
}

static bool ignoreHeader(const std::string& header, const std::string& expect) {
    return (expect.length() == header.length()) &&
           std::equal(header.begin(), header.end(), expect.begin(),
                      [](char a, char b) { return ::tolower(a) == ::tolower(b); });
}

CurlMultiTransport::CurlMultiTransport(const struct HttpTransportOptions& options) {
    if (options.connectTimeout.has_value()) {
        connectTimeout_ = options.connectTimeout.value();
    }
    if (options.readWriteTimeout.has_value()) {
        requestTimeout_ = options.readWriteTimeout.value();
    }
    if (options.insecureSkipVerify.has_value()) {
        verifySSL_ = !options.insecureSkipVerify.value();
    }
    if (options.proxyHost.has_value()) {
        proxyHost_ = options.proxyHost.value();
    }

    multiHandle_ = curl_multi_init();
    if (multiHandle_ == nullptr) {
        throw std::runtime_error("curl_multi_init failed");
    }

    ioThread_ = std::thread([this]() { ioLoop(); });
}

CurlMultiTransport::~CurlMultiTransport() {
    stopped_.store(true, std::memory_order_release);

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 68, 0)
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
    if (stopped_.load(std::memory_order_acquire)) {
        callback(std::make_error_code(std::errc::operation_canceled),
                 std::move(request), std::move(context));
        return;
    }

    auto ctx = std::make_unique<AsyncTransferContext>();
    ctx->request = std::move(request);
    ctx->context = std::move(context);
    ctx->callback = std::move(callback);
    ctx->response = std::make_unique<ResponseMessage>();

    if (ctx->request->body != nullptr) {
        ctx->source = ctx->request->body->spanSource();
    }
    ctx->recvFirstData = true;
    ctx->recvDataLength = -1;

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pendingRequests_.push_back(std::move(ctx));
    }

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 68, 0)
    curl_multi_wakeup(multiHandle_);
#endif
}

void CurlMultiTransport::cleanupTransferContext(AsyncTransferContext* ctx) {
    if (ctx->headers) {
        curl_slist_free_all(ctx->headers);
    }
    if (ctx->curl) {
        curl_easy_cleanup(ctx->curl);
    }
}

void CurlMultiTransport::setupCurlHandle(AsyncTransferContext* ctx) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        return;
    }
    ctx->curl = curl;

    curl_slist* list = nullptr;
    for (const auto& [k, v] : ctx->request->headers) {
        if (v.empty()) continue;
        if (ignoreHeader(k, "Content-Length")) continue;
        std::string str = k;
        str.append(": ").append(v);
        list = curl_slist_append(list, str.c_str());
    }
    list = curl_slist_append(list, "Expect:");
    ctx->headers = list;

    int64_t contentlength = -1;
    if (ctx->request->headers.find("Content-Length") != ctx->request->headers.end()) {
        auto& str = ctx->request->headers.at("Content-Length");
        long long result = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
        if (ec == std::errc()) {
            contentlength = result;
        }
    }
    if (contentlength < 0 && ctx->request->body != nullptr && ctx->request->body->length().has_value()) {
        contentlength = static_cast<int64_t>(ctx->request->body->length().value());
    }
    if (contentlength >= 0) {
        std::string str = "Content-Length: ";
        str.append(std::to_string(contentlength));
        list = curl_slist_append(list, str.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, ctx->request->uri.c_str());

    if ("HEAD" == ctx->request->method) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    } else if ("PUT" == ctx->request->method) {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        if (ctx->request->body == nullptr) {
            curl_off_t len = 0;
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);
        } else if (contentlength >= 0) {
            curl_off_t len = static_cast<curl_off_t>(contentlength);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);
        }
    } else if ("POST" == ctx->request->method) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (ctx->request->body == nullptr) {
            curl_off_t len = 0;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, len);
        } else if (contentlength >= 0) {
            curl_off_t len = static_cast<curl_off_t>(contentlength);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, len);
        }
    } else if ("DELETE" == ctx->request->method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, ctx);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeaders);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctx);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBody);
    curl_easy_setopt(curl, CURLOPT_READDATA, ctx);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBody);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
    curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout_);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, requestTimeout_ / 1000);

    if (verifySSL_) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (!proxyHost_.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyHost_.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long) proxyPort_);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, proxyUserName_.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, proxyPassword_.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_PRIVATE, ctx);
}

void CurlMultiTransport::drainPending() {
    std::vector<std::unique_ptr<AsyncTransferContext>> batch;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        batch.swap(pendingRequests_);
    }

    for (auto& ctx : batch) {
        setupCurlHandle(ctx.get());
        if (ctx->curl == nullptr) {
            auto callback = std::move(ctx->callback);
            callback(std::make_error_code(std::errc::resource_unavailable_try_again),
                     std::move(ctx->request), std::move(ctx->context));
            continue;
        }
        curl_multi_add_handle(multiHandle_, ctx->curl);
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
        if (raw == nullptr) {
            curl_multi_remove_handle(multiHandle_, curl);
            curl_easy_cleanup(curl);
            continue;
        }

        std::unique_ptr<AsyncTransferContext> ctx(raw);
        curl_multi_remove_handle(multiHandle_, curl);

        long response_code = 0;
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        } else {
            std::stringstream ss;
            ss << curl_easy_strerror(res);
            if (res == CURLE_WRITE_ERROR) {
                if (ctx->sink == nullptr) {
                    ss << ". Caused by sink is null.";
                } else if (ctx->sink->bad()) {
                    ss << ". Caused by sink is in bad state.";
                } else if (ctx->sink->fail()) {
                    ss << ". Caused by sink is in fail state.";
                }
            }
            ctx->context.errorCode = "CURLcode " + std::to_string(res);
            ctx->context.errorMessage = ss.str();
        }

        ctx->response->statusCode = response_code;
        ctx->response->body = ctx->defaultSink;

        curl_slist_free_all(ctx->headers);
        curl_easy_cleanup(curl);
        ctx->curl = nullptr;
        ctx->headers = nullptr;

        OSS_LOG(LogLevel::LogDebug, TAG, "completed async request, CURLcode:%d, ResponseCode:%d",
                res, response_code);

        if (res != CURLE_OK) {
            auto callback = std::move(ctx->callback);
            callback(std::make_error_code(std::errc::io_error),
                     std::move(ctx->request), std::move(ctx->context));
        } else {
            auto callback = std::move(ctx->callback);
            auto response = std::move(ctx->response);
            callback(std::move(response),
                     std::move(ctx->request), std::move(ctx->context));
        }
    }
}

void CurlMultiTransport::cleanupInflight() {
    // Cancel all pending (not yet submitted) requests
    std::vector<std::unique_ptr<AsyncTransferContext>> pending;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pending.swap(pendingRequests_);
    }
    for (auto& ctx : pending) {
        auto callback = std::move(ctx->callback);
        callback(std::make_error_code(std::errc::operation_canceled),
                 std::move(ctx->request), std::move(ctx->context));
    }

    // Cancel all in-flight requests still in the multi handle
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 84, 0)
    CURL** handles = curl_multi_get_handles(multiHandle_);
    if (handles) {
        for (int i = 0; handles[i] != nullptr; i++) {
            CURL* curl = handles[i];
            AsyncTransferContext* raw = nullptr;
            curl_easy_getinfo(curl, CURLINFO_PRIVATE, &raw);
            curl_multi_remove_handle(multiHandle_, curl);
            if (raw) {
                std::unique_ptr<AsyncTransferContext> ctx(raw);
                auto callback = std::move(ctx->callback);
                cleanupTransferContext(ctx.get());
                ctx->curl = nullptr;
                ctx->headers = nullptr;
                callback(std::make_error_code(std::errc::operation_canceled),
                         std::move(ctx->request), std::move(ctx->context));
            } else {
                curl_easy_cleanup(curl);
            }
        }
        curl_free(handles);
    }
#else
    CURLMsg* msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(multiHandle_, &msgs_left)) != nullptr) {
        if (msg->msg == CURLMSG_DONE) {
            CURL* curl = msg->easy_handle;
            AsyncTransferContext* raw = nullptr;
            curl_easy_getinfo(curl, CURLINFO_PRIVATE, &raw);
            curl_multi_remove_handle(multiHandle_, curl);
            if (raw) {
                std::unique_ptr<AsyncTransferContext> ctx(raw);
                auto callback = std::move(ctx->callback);
                cleanupTransferContext(ctx.get());
                ctx->curl = nullptr;
                ctx->headers = nullptr;
                callback(std::make_error_code(std::errc::operation_canceled),
                         std::move(ctx->request), std::move(ctx->context));
            } else {
                curl_easy_cleanup(curl);
            }
        }
    }
#endif
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
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 66, 0)
        curl_multi_poll(multiHandle_, nullptr, 0, 200, &numfds);
#else
        curl_multi_wait(multiHandle_, nullptr, 0, 200, &numfds);
#endif
    }

    // Final drain of completed requests
    int still_running = 0;
    curl_multi_perform(multiHandle_, &still_running);
    processCompleted();

    // Cancel any remaining in-flight and pending requests
    cleanupInflight();

    OSS_LOG(LogLevel::LogInfo, TAG, "IO loop stopped");
}

} // namespace alibabacloud::oss2::transport::curl
