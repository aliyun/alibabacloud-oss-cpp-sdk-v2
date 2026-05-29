#include "WinHttpHelper.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/LogUtils.h"

#include <charconv>
#include <sstream>

namespace alibabacloud::oss2::transport::winhttp {

std::wstring toWideString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
    int size =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    if (size == 0) {
        return std::wstring();
    }
    std::wstring wstr(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
    return wstr;
}

std::string fromWideString(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }
    int size =
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        return std::string();
    }
    std::string str(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], size, nullptr, nullptr);
    return str;
}

std::string formatWinHttpError(DWORD winError) {
    std::string msg = "WinHTTP error " + std::to_string(winError);
    char* buf = nullptr;
    DWORD len =
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                       GetModuleHandleA("winhttp.dll"), winError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPSTR>(&buf), 0, nullptr);
    if (len != 0 && buf != nullptr) {
        msg += ": ";
        msg.append(buf, len);
        while (!msg.empty() && (msg.back() == '\r' || msg.back() == '\n')) {
            msg.pop_back();
        }
        LocalFree(buf);
    }
    return msg;
}

TransportError makeWinHttpError(TransportErrorCode code, DWORD winError) {
    return TransportError{make_error_code(code), "WinHttpError", formatWinHttpError(winError)};
}

ConnectionOptions buildConnectionOptions(const HttpTransportOptions& options) {
    ConnectionOptions opts;
    opts.verifySSL = !options.insecureSkipVerify.value_or(false);
    opts.enabledRedirect = options.enabledRedirect.value_or(false);
    if (options.proxyHost.has_value() && !options.proxyHost.value().empty()) {
        opts.proxyHost = options.proxyHost.value();
    }
    return opts;
}

ConnectionOptions buildConnectionOptions(const WinHttpTransportOptions& options) {
    ConnectionOptions opts = buildConnectionOptions(static_cast<const HttpTransportOptions&>(options));
    if (options.proxyPort.has_value()) {
        opts.proxyPort = options.proxyPort.value();
    }
    if (options.proxyUserName.has_value()) {
        opts.proxyUserName = options.proxyUserName.value();
    }
    if (options.proxyPassword.has_value()) {
        opts.proxyPassword = options.proxyPassword.value();
    }
    return opts;
}

HeaderMap parseResponseHeaders(const std::string& rawHeaders) {
    HeaderMap headers;
    std::istringstream stream(rawHeaders);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        auto pos = line.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        auto name = line.substr(0, pos);
        size_t valueStart = pos + 1;
        while (valueStart < line.size() && (line[valueStart] == ' ' || line[valueStart] == '\t')) {
            ++valueStart;
        }
        size_t valueEnd = line.size();
        while (valueEnd > valueStart && (line[valueEnd - 1] == ' ' || line[valueEnd - 1] == '\t')) {
            --valueEnd;
        }
        headers.emplace(std::move(name), line.substr(valueStart, valueEnd - valueStart));
    }
    return headers;
}

static const char* TAG = "WinHttpHelper";

WinHttpHandle openSession(const ConnectionOptions& connOpts, unsigned int maxConnsPerServer, long connectTimeout,
                          long requestTimeout) {
    DWORD accessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
    LPCWSTR proxyName = WINHTTP_NO_PROXY_NAME;
    std::wstring wProxyHost;

    if (!connOpts.proxyHost.empty()) {
        accessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        wProxyHost = toWideString(connOpts.proxyHost);
        proxyName = wProxyHost.c_str();
    }

    WinHttpHandle hSession(WinHttpOpen(L"alibabacloud-oss-cpp-sdk-v2", accessType, proxyName, WINHTTP_NO_PROXY_BYPASS,
                                       WINHTTP_FLAG_ASYNC));

    if (!hSession) {
        OSS_LOG(LogLevel::LogError, TAG, "Failed to open WinHttp session, error: %lu", GetLastError());
        return hSession;
    }

    WinHttpSetTimeouts(hSession.get(), connectTimeout, connectTimeout, requestTimeout, requestTimeout);

    DWORD tlsFlags = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
#if defined(WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3)
    tlsFlags |= WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
#endif
    WinHttpSetOption(hSession.get(), WINHTTP_OPTION_SECURE_PROTOCOLS, &tlsFlags, sizeof(tlsFlags));

    DWORD maxConns = static_cast<DWORD>(maxConnsPerServer);
    WinHttpSetOption(hSession.get(), WINHTTP_OPTION_MAX_CONNS_PER_SERVER, &maxConns, sizeof(maxConns));

    return hSession;
}

std::optional<TransportError> openRequest(HINTERNET hSession, const std::string& uri, const std::string& method,
                                          RequestHandles& out) {
    URL_COMPONENTS urlComp;
    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength = static_cast<DWORD>(-1);
    urlComp.dwHostNameLength = static_cast<DWORD>(-1);
    urlComp.dwUrlPathLength = static_cast<DWORD>(-1);
    urlComp.dwExtraInfoLength = static_cast<DWORD>(-1);

    std::wstring wUrl = toWideString(uri);
    if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
        return TransportError{make_error_code(ClientErrorCode::EndpointInvalid), "UrlParseError",
                              "Failed to parse URL: " + uri};
    }

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    uint16_t port = urlComp.nPort;
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
    if (urlComp.dwExtraInfoLength > 0) {
        path.append(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
    }

    bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    out.hConnect.reset(WinHttpConnect(hSession, host.c_str(), port, 0));
    if (!out.hConnect) {
        return makeWinHttpError(TransportErrorCode::ConnectionFailed, GetLastError());
    }

    DWORD requestFlags = 0;
    if (isHttps) {
        requestFlags |= WINHTTP_FLAG_SECURE;
    }

    std::wstring wMethod = toWideString(method);
    out.hRequest.reset(WinHttpOpenRequest(out.hConnect.get(), wMethod.c_str(), path.c_str(), nullptr,
                                          WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags));

    if (!out.hRequest) {
        return makeWinHttpError(TransportErrorCode::ConnectionFailed, GetLastError());
    }

    return std::nullopt;
}

void applyRequestOptions(HINTERNET hRequest, const ConnectionOptions& connOpts) {
    if (!connOpts.verifySSL) {
        DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID
            | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));
    }

    if (!connOpts.proxyUserName.empty()) {
        std::wstring wUser = toWideString(connOpts.proxyUserName);
        std::wstring wPass = toWideString(connOpts.proxyPassword);
        WinHttpSetCredentials(hRequest, WINHTTP_AUTH_TARGET_PROXY, WINHTTP_AUTH_SCHEME_BASIC, wUser.c_str(),
                              wPass.c_str(), nullptr);
    }

    if (!connOpts.enabledRedirect) {
        DWORD disableFlags = WINHTTP_DISABLE_REDIRECTS;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disableFlags, sizeof(disableFlags));
    }
}

int64_t resolveContentLength(const HeaderCollection& headers, const std::shared_ptr<ByteContent>& body) {
    int64_t contentLength = -1;
    auto clIt = headers.find("Content-Length");
    if (clIt != headers.end()) {
        long long result = 0;
        auto [ptr, ec] = std::from_chars(clIt->second.data(), clIt->second.data() + clIt->second.size(), result);
        if (ec == std::errc()) {
            contentLength = result;
        }
    }
    if (contentLength < 0 && body != nullptr && body->length().has_value()) {
        contentLength = static_cast<int64_t>(body->length().value());
    }
    return contentLength;
}

void addRequestHeaders(HINTERNET hRequest, const HeaderCollection& headers) {
    std::string narrowHeaders;
    for (const auto& [k, v] : headers) {
        if (v.empty()) {
            continue;
        }
        narrowHeaders.append(k);
        narrowHeaders.append(": ");
        narrowHeaders.append(v);
        narrowHeaders.append("\r\n");
    }
    if (!narrowHeaders.empty()) {
        std::wstring wHeaders = toWideString(narrowHeaders);
        WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), static_cast<DWORD>(wHeaders.length()),
                                 WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD);
    }
}

void readResponseStatusAndHeaders(HINTERNET hRequest, ResponseMessage& response) {
    {
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
        response.statusCode = static_cast<long>(statusCode);
    }

    {
        DWORD headerSize = 0;
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, nullptr,
                            &headerSize, WINHTTP_NO_HEADER_INDEX);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && headerSize > 0) {
            std::vector<wchar_t> headerBuf(headerSize / sizeof(wchar_t));
            if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
                                    headerBuf.data(), &headerSize, WINHTTP_NO_HEADER_INDEX)) {
                std::string rawHeaders = fromWideString(std::wstring(headerBuf.data()));
                auto parsed = parseResponseHeaders(rawHeaders);
                for (auto& [k, v] : parsed) {
                    response.headers.emplace(std::move(k), std::move(v));
                }
            }
        }
    }
}

ResponseSink createResponseSink(long statusCode, const std::optional<SinkFactory>& factory,
                                const HeaderCollection& headers) {
    ResponseSink rs;
    bool isError = (statusCode / 100 != 2) || (statusCode == 203);

    if (!isError && factory.has_value()) {
        int64_t contentLength = -1;
        auto it = headers.find("Content-Length");
        if (it != headers.end()) {
            long long val = 0;
            auto [ptr, ec] = std::from_chars(it->second.data(), it->second.data() + it->second.size(), val);
            if (ec == std::errc()) {
                contentLength = val;
            }
        }
        rs.sink = factory.value()(contentLength, headers);
    }
    if (!rs.sink) {
        rs.defaultSink = std::make_shared<std::stringstream>();
        rs.sink = std::make_shared<OStreamWriter>(rs.defaultSink);
    }
    return rs;
}

void finalizeResponseBody(ResponseMessage& response, long statusCode, const std::optional<SinkFactory>& factory,
                          const std::shared_ptr<std::stringstream>& defaultSink) {
    bool isError = (statusCode / 100 != 2) || (statusCode == 203);
    if (isError || !factory.has_value()) {
        response.body = defaultSink;
    }
}

} // namespace alibabacloud::oss2::transport::winhttp
