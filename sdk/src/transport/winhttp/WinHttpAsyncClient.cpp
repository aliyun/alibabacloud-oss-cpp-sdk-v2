#include "WinHttpAsyncClient.h"
#include "alibabacloud/oss2/Error.h"
#include "src/transport/TransportDefaults.h"
#include "src/utils/LogUtils.h"

namespace alibabacloud::oss2::transport::winhttp {

static const char* TAG = "WinHttpAsyncClient";

struct AsyncRequestContext {
    WinHttpAsyncClient* client{nullptr};

    std::unique_ptr<RequestMessage> request;
    RequestCallback callback;
    std::unique_ptr<ResponseMessage> response;
    RequestOptions options;

    RequestHandles handles;

    std::unique_ptr<ByteSource> source;
    int64_t contentLength{-1};

    ResponseSink rs;

    char buffer[kWriteBufferLength];

    enum class State { SendRequest, WriteBody, ReceiveResponse, QueryData, ReadData, CloseHandle };
    State state{State::SendRequest};

    std::optional<TransportError> error;

    bool checkCancelled() {
        if (options.cancellationToken.has_value() && options.cancellationToken->isCanceled()) {
            error = TransportError{make_error_code(TransportErrorCode::Canceled), "RequestCanceled",
                                   "Request canceled by CancellationToken"};
            closeHandle();
            return true;
        }
        return false;
    }

    void closeHandle() {
        state = State::CloseHandle;
        if (handles.hRequest) {
            WinHttpCloseHandle(handles.hRequest.release());
        } else {
            finish();
        }
    }

    void finish() {
        handles.hConnect.reset();
        ResponseResult result;
        if (error.has_value()) {
            result = std::move(error.value());
        } else {
            result = std::move(response);
        }
        auto cb = std::move(callback);
        auto req = std::move(request);
        auto* owner = client;
        delete this;
        struct InflightGuard {
            WinHttpAsyncClient* o;
            ~InflightGuard() {
                o->onRequestFinished();
            }
        } guard{owner};
        cb(std::move(result), std::move(req));
    }

    void failWithWinHttpError(TransportErrorCode code, DWORD winError) {
        error = makeWinHttpError(code, winError);
        closeHandle();
    }

    // --- state machine handlers ---

    void onSendComplete() {
        if (checkCancelled()) {
            return;
        }

        if (source != nullptr && contentLength > 0) {
            state = State::WriteBody;
            writeNextChunk();
        } else {
            receiveResponse();
        }
    }

    void writeNextChunk() {
        size_t got = source->readToCount(reinterpret_cast<uint8_t*>(buffer), kWriteBufferLength);
        if (got < kWriteBufferLength) {
            auto st = source->state();
            if (st != 0 && (st & std::ios::eofbit) == 0) {
                error = TransportError{make_error_code(TransportErrorCode::SendRecvError), "ReadBodyError",
                                       "Failed to read request body"};
                closeHandle();
                return;
            }
        }
        if (got == 0) {
            receiveResponse();
            return;
        }

        if (!WinHttpWriteData(handles.hRequest.get(), buffer, static_cast<DWORD>(got), nullptr)) {
            failWithWinHttpError(TransportErrorCode::SendRecvError, GetLastError());
        }
    }

    void onWriteComplete() {
        if (checkCancelled()) {
            return;
        }
        writeNextChunk();
    }

    void receiveResponse() {
        state = State::ReceiveResponse;
        if (!WinHttpReceiveResponse(handles.hRequest.get(), nullptr)) {
            failWithWinHttpError(TransportErrorCode::SendRecvError, GetLastError());
        }
    }

    void onHeadersAvailable() {
        if (checkCancelled()) {
            return;
        }

        readResponseStatusAndHeaders(handles.hRequest.get(), *response);

        rs = createResponseSink(response->statusCode, options.sinkFactory, response->headers);

        queryData();
    }

    void queryData() {
        state = State::QueryData;
        if (!WinHttpQueryDataAvailable(handles.hRequest.get(), nullptr)) {
            failWithWinHttpError(TransportErrorCode::SendRecvError, GetLastError());
        }
    }

    void onDataAvailable(DWORD bytesAvailable) {
        if (checkCancelled()) {
            return;
        }

        if (bytesAvailable == 0) {
            finalizeResponseBody(*response, response->statusCode, options.sinkFactory, rs.defaultSink);
            closeHandle();
            return;
        }

        state = State::ReadData;
        DWORD toRead = (std::min)(bytesAvailable, static_cast<DWORD>(kWriteBufferLength));
        if (!WinHttpReadData(handles.hRequest.get(), buffer, toRead, nullptr)) {
            failWithWinHttpError(TransportErrorCode::SendRecvError, GetLastError());
        }
    }

    void onReadComplete(DWORD bytesRead) {
        if (checkCancelled()) {
            return;
        }

        if (bytesRead == 0) {
            finalizeResponseBody(*response, response->statusCode, options.sinkFactory, rs.defaultSink);
            closeHandle();
            return;
        }

        rs.sink->write(reinterpret_cast<const std::uint8_t*>(buffer), bytesRead);
        if (rs.sink->bad()) {
            error = TransportError{make_error_code(TransportErrorCode::SendRecvError), "WriteStreamError",
                                   "Failed to write response body to output stream"};
            closeHandle();
            return;
        }

        queryData();
    }

    void onError(WINHTTP_ASYNC_RESULT* asyncResult) {
        OSS_LOG(LogLevel::LogError, TAG, "Async request error, API: %lu, error: %lu",
                static_cast<unsigned long>(asyncResult->dwResult), asyncResult->dwError);

        DWORD winErr = asyncResult->dwError;
        if (winErr == ERROR_WINHTTP_TIMEOUT) {
            error = makeWinHttpError(TransportErrorCode::Timeout, winErr);
        } else {
            error = makeWinHttpError(TransportErrorCode::SendRecvError, winErr);
        }
        closeHandle();
    }

    void onHandleClosing() {
        finish();
    }

    void onStatus(DWORD internetStatus, LPVOID statusInfo, DWORD statusInfoLength) {
        if (internetStatus == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR) {
            onError(static_cast<WINHTTP_ASYNC_RESULT*>(statusInfo));
            return;
        }

        if (internetStatus == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING) {
            onHandleClosing();
            return;
        }

        switch (state) {
            case State::SendRequest:
                if (internetStatus == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE) {
                    onSendComplete();
                }
                break;
            case State::WriteBody:
                if (internetStatus == WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE) {
                    onWriteComplete();
                }
                break;
            case State::ReceiveResponse:
                if (internetStatus == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE) {
                    onHeadersAvailable();
                }
                break;
            case State::QueryData:
                if (internetStatus == WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE) {
                    onDataAvailable(statusInfo ? *static_cast<DWORD*>(statusInfo) : 0);
                }
                break;
            case State::ReadData:
                if (internetStatus == WINHTTP_CALLBACK_STATUS_READ_COMPLETE) {
                    onReadComplete(statusInfoLength);
                }
                break;
            case State::CloseHandle: break;
        }
    }

    static void CALLBACK statusCallback(HINTERNET /*hInternet*/, DWORD_PTR dwContext, DWORD dwInternetStatus,
                                        LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
        if (dwContext == 0) {
            return;
        }
        auto* ctx = reinterpret_cast<AsyncRequestContext*>(dwContext);
        ctx->onStatus(dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
    }

    bool start(HINTERNET hSession, const ConnectionOptions& connOpts) {
        response = std::make_unique<ResponseMessage>();

        auto err = openRequest(hSession, request->uri, request->method, handles);
        if (err.has_value()) {
            error = std::move(err.value());
            return false;
        }

        if (WinHttpSetStatusCallback(handles.hRequest.get(), &AsyncRequestContext::statusCallback,
                                     WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES
                                         | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR,
                                     0)
            == WINHTTP_INVALID_STATUS_CALLBACK) {
            error = makeWinHttpError(TransportErrorCode::ConnectionFailed, GetLastError());
            return false;
        }

        applyRequestOptions(handles.hRequest.get(), connOpts);

        contentLength = resolveContentLength(request->headers, request->body);

        if (request->body != nullptr && contentLength > 0) {
            source = request->body->spanSource();
        }

        addRequestHeaders(handles.hRequest.get(), request->headers);

        DWORD totalLength = WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH;
        if (contentLength >= 0 && contentLength <= static_cast<int64_t>(MAXDWORD)) {
            totalLength = static_cast<DWORD>(contentLength);
        }

        auto contextPtr = reinterpret_cast<DWORD_PTR>(this);
        state = State::SendRequest;

        if (!WinHttpSendRequest(handles.hRequest.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0,
                                totalLength, contextPtr)) {
            error = makeWinHttpError(TransportErrorCode::SendRecvError, GetLastError());
            return false;
        }

        return true;
    }
};

// --- WinHttpAsyncClient implementation ---

WinHttpAsyncClient::WinHttpAsyncClient(const HttpTransportOptions& options) {
    connOpts_ = buildConnectionOptions(options);
    long connectTimeout = options.connectTimeout.value_or(kDefaultConnectTimeoutMs);
    long requestTimeout = options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs);
    hSession_ = openSession(connOpts_, kDefaultMaxConnectionsSync, connectTimeout, requestTimeout);
}

WinHttpAsyncClient::WinHttpAsyncClient(const WinHttpTransportOptions& options) {
    connOpts_ = buildConnectionOptions(options);
    long connectTimeout = options.connectTimeout.value_or(kDefaultConnectTimeoutMs);
    long requestTimeout = options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs);
    unsigned int maxConns = options.maxConnections.value_or(kDefaultMaxConnectionsSync);
    hSession_ = openSession(connOpts_, maxConns, connectTimeout, requestTimeout);
}

WinHttpAsyncClient::~WinHttpAsyncClient() {
    hSession_.reset();
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return inflightCount_.load() == 0; });
}

void WinHttpAsyncClient::onRequestStarted() {
    inflightCount_.fetch_add(1, std::memory_order_relaxed);
}

void WinHttpAsyncClient::onRequestFinished() {
    if (inflightCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
}

void WinHttpAsyncClient::sendAsync(std::unique_ptr<RequestMessage> request, const RequestOptions& options,
                                   RequestCallback callback) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter sendAsync", request.get());

    if (!hSession_) {
        callback(TransportError{make_error_code(TransportErrorCode::ConnectionFailed), "WinHttpError",
                                "WinHttp session is not initialized"},
                 std::move(request));
        return;
    }

    if (options.cancellationToken.has_value() && options.cancellationToken->isCanceled()) {
        callback(TransportError{make_error_code(TransportErrorCode::Canceled), "RequestCanceled",
                                "Request canceled by CancellationToken"},
                 std::move(request));
        return;
    }

    auto* ctx = new AsyncRequestContext();
    ctx->client = this;
    ctx->request = std::move(request);
    ctx->callback = std::move(callback);
    ctx->options = options;

    onRequestStarted();

    if (!ctx->start(hSession_.get(), connOpts_)) {
        auto err = std::move(ctx->error.value());
        auto req = std::move(ctx->request);
        auto cb = std::move(ctx->callback);
        delete ctx;
        struct InflightGuard {
            WinHttpAsyncClient* o;
            ~InflightGuard() {
                o->onRequestFinished();
            }
        } guard{this};
        cb(std::move(err), std::move(req));
    }
}

} // namespace alibabacloud::oss2::transport::winhttp
