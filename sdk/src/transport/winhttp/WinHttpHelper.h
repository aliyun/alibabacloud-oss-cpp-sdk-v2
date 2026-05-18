#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportOptions.h"

#include <windows.h>
#include <winhttp.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace alibabacloud::oss2::transport::winhttp {

struct WinHttpHandleDeleter {
    void operator()(void* handle) const {
        WinHttpCloseHandle(static_cast<HINTERNET>(handle));
    }
};
using WinHttpHandle = std::unique_ptr<void, WinHttpHandleDeleter>;

std::wstring toWideString(const std::string& str);
std::string fromWideString(const std::wstring& wstr);

std::string formatWinHttpError(DWORD winError);
TransportError makeWinHttpError(TransportErrorCode code, DWORD winError);

struct ConnectionOptions {
    bool verifySSL{true};
    bool enabledRedirect{false};
    std::string proxyHost;
    unsigned int proxyPort{};
    std::string proxyUserName;
    std::string proxyPassword;
};

ConnectionOptions buildConnectionOptions(const HttpTransportOptions& options);
ConnectionOptions buildConnectionOptions(const WinHttpTransportOptions& options);

using HeaderMap = std::map<std::string, std::string>;
HeaderMap parseResponseHeaders(const std::string& rawHeaders);

// --- Shared helpers for sync/async clients ---

WinHttpHandle openSession(const ConnectionOptions& connOpts,
                           unsigned int maxConnsPerServer,
                           long connectTimeout, long requestTimeout);

struct RequestHandles {
    WinHttpHandle hConnect;
    WinHttpHandle hRequest;
};

std::optional<TransportError> openRequest(HINTERNET hSession,
                                           const std::string& uri,
                                           const std::string& method,
                                           RequestHandles& out);

void applyRequestOptions(HINTERNET hRequest, const ConnectionOptions& connOpts);

int64_t resolveContentLength(const HeaderCollection& headers,
                              const std::shared_ptr<ByteContent>& body);

void addRequestHeaders(HINTERNET hRequest, const HeaderCollection& headers);

void readResponseStatusAndHeaders(HINTERNET hRequest, ResponseMessage& response);

struct ResponseSink {
    std::shared_ptr<std::ostream> sink;
    std::shared_ptr<std::stringstream> defaultSink;
};

int64_t parseResponseContentLength(const HeaderCollection& headers);

ResponseSink createResponseSink(long statusCode,
                                 const std::optional<OStreamFactory>& factory,
                                 int64_t contentLength);

void finalizeResponseBody(ResponseMessage& response, long statusCode,
                           const std::optional<OStreamFactory>& factory,
                           const std::shared_ptr<std::stringstream>& defaultSink);

} // namespace alibabacloud::oss2::transport::winhttp
