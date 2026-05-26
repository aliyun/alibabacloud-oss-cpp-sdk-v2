#include "WinHttpClient.h"
#include "WinHttpAction.h"
#include "alibabacloud/oss2/Error.h"
#include "src/transport/TransportDefaults.h"
#include "src/utils/LogUtils.h"

namespace alibabacloud::oss2::transport::winhttp {

namespace {
struct AsyncHandleGuard {
    WinHttpHandle& handle;
    WinHttpAction& action;
    ~AsyncHandleGuard() {
        if (handle) {
            action.waitForAction(
                [this]() { WinHttpCloseHandle(handle.release()); return true; },
                WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING,
                std::nullopt);
        }
    }
};
} // namespace

static const char* TAG = "WinHttpClient";

WinHttpClient::WinHttpClient(const HttpTransportOptions& options)
    : isRequestDisabled_(options.isRequestDisabled) {
    connOpts_ = buildConnectionOptions(options);
    long connectTimeout = options.connectTimeout.value_or(kDefaultConnectTimeoutMs);
    long requestTimeout = options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs);
    hSession_ = openSession(connOpts_, kDefaultMaxConnectionsSync, connectTimeout, requestTimeout);
}

WinHttpClient::WinHttpClient(const WinHttpTransportOptions& options)
    : isRequestDisabled_(options.isRequestDisabled) {
    connOpts_ = buildConnectionOptions(options);
    long connectTimeout = options.connectTimeout.value_or(kDefaultConnectTimeoutMs);
    long requestTimeout = options.readWriteTimeout.value_or(kDefaultReadWriteTimeoutMs);
    unsigned int maxConns = options.maxConnections.value_or(kDefaultMaxConnectionsSync);
    hSession_ = openSession(connOpts_, maxConns, connectTimeout, requestTimeout);
}

WinHttpClient::~WinHttpClient() = default;

ResponseResult WinHttpClient::send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter Send", request.get());

    if (!hSession_) {
        return TransportError{make_error_code(TransportErrorCode::ConnectionFailed),
                              "WinHttpError", "WinHttp session is not initialized"};
    }

    auto response = std::make_unique<ResponseMessage>();

    RequestHandles handles;
    auto err = openRequest(hSession_.get(), request->uri, request->method, handles);
    if (err.has_value()) {
        return std::move(err.value());
    }

    WinHttpAction action;
    if (!action.registerCallback(handles.hRequest.get())) {
        return makeWinHttpError(TransportErrorCode::ConnectionFailed, GetLastError());
    }
    AsyncHandleGuard requestGuard{handles.hRequest, action};

    auto isAborted = [&]() {
        if (options.cancellationToken.has_value() && options.cancellationToken->isCanceled()) {
            return true;
        }
        if (isRequestDisabled_ && isRequestDisabled_()) {
            return true;
        }
        return false;
    };

    auto makeWaitError = [&]() -> TransportError {
        if (isAborted()) {
            return TransportError{make_error_code(TransportErrorCode::Canceled),
                                  "RequestCanceled", "Request canceled by CancellationToken"};
        }
        DWORD winErr = action.getError();
        if (winErr == ERROR_WINHTTP_TIMEOUT) {
            return makeWinHttpError(TransportErrorCode::Timeout, winErr);
        }
        return makeWinHttpError(TransportErrorCode::SendRecvError, winErr);
    };

    applyRequestOptions(handles.hRequest.get(), connOpts_);

    int64_t contentLength = resolveContentLength(request->headers, request->body);

    addRequestHeaders(handles.hRequest.get(), request->headers);

    DWORD totalLength = WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH;
    if (contentLength >= 0 && contentLength <= static_cast<int64_t>(MAXDWORD)) {
        totalLength = static_cast<DWORD>(contentLength);
    }

    auto actionPtr = reinterpret_cast<DWORD_PTR>(&action);

    if (!action.waitForAction(
            [&]() {
                return WinHttpSendRequest(handles.hRequest.get(),
                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0,
                    totalLength, actionPtr) != 0;
            },
            WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE,
            options.cancellationToken,
            isRequestDisabled_)) {
        return makeWaitError();
    }

    if (request->body != nullptr && contentLength > 0) {
        auto source = request->body->spanSource();
        if (source != nullptr) {
            char buffer[kWriteBufferLength];
            for (;;) {
                size_t got = source->readToCount(reinterpret_cast<uint8_t*>(buffer), kWriteBufferLength);
                if (got < kWriteBufferLength) {
                    auto st = source->state();
                    if (st != 0 && (st & std::ios::eofbit) == 0) {
                        return TransportError{make_error_code(TransportErrorCode::SendRecvError),
                                              "ReadBodyError", "Failed to read request body"};
                    }
                }
                if (got == 0) break;

                if (!action.waitForAction(
                        [&]() {
                            return WinHttpWriteData(handles.hRequest.get(), buffer,
                                static_cast<DWORD>(got), nullptr) != 0;
                        },
                        WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE,
                        options.cancellationToken,
                        isRequestDisabled_)) {
                    return makeWaitError();
                }
            }
        }
    }

    if (!action.waitForAction(
            [&]() {
                return WinHttpReceiveResponse(handles.hRequest.get(), nullptr) != 0;
            },
            WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE,
            options.cancellationToken,
            isRequestDisabled_)) {
        return makeWaitError();
    }

    readResponseStatusAndHeaders(handles.hRequest.get(), *response);

    {
        auto rs = createResponseSink(response->statusCode, options.sinkFactory, response->headers);

        char readBuf[kWriteBufferLength];
        for (;;) {
            if (!action.waitForAction(
                    [&]() {
                        return WinHttpQueryDataAvailable(handles.hRequest.get(), nullptr) != 0;
                    },
                    WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE,
                    options.cancellationToken,
                    isRequestDisabled_)) {
                if (isAborted()) return makeWaitError();
                break;
            }

            DWORD available = action.getBytesAvailable();
            if (available == 0) break;

            while (available > 0) {
                DWORD toRead = (std::min)(available, static_cast<DWORD>(kWriteBufferLength));
                if (!action.waitForAction(
                        [&]() {
                            return WinHttpReadData(handles.hRequest.get(), readBuf, toRead, nullptr) != 0;
                        },
                        WINHTTP_CALLBACK_STATUS_READ_COMPLETE,
                        options.cancellationToken,
                        isRequestDisabled_)) {
                    if (isAborted()) return makeWaitError();
                    available = 0;
                    break;
                }

                DWORD bytesRead = action.getBytesRead();
                if (bytesRead == 0) {
                    available = 0;
                    break;
                }
                rs.sink->write(reinterpret_cast<const std::uint8_t*>(readBuf), bytesRead);
                if (rs.sink->bad()) {
                    return TransportError{make_error_code(TransportErrorCode::SendRecvError),
                                          "WriteStreamError", "Failed to write response body to output stream"};
                }
                available -= bytesRead;
            }
        }

        finalizeResponseBody(*response, response->statusCode, options.sinkFactory, rs.defaultSink);
    }

    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, ResponseCode:%ld",
            request.get(), response->statusCode);

    return response;
}

} // namespace alibabacloud::oss2::transport::winhttp
