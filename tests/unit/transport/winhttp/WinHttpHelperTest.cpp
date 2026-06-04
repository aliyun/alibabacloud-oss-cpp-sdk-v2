#include <gtest/gtest.h>

#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportFactory.h"
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportOptions.h"
#include "src/transport/TransportDefaults.h"
#include "src/transport/winhttp/WinHttpHelper.h"

namespace alibabacloud::oss2::transport::winhttp {

// ---------- toWideString ----------

TEST(WinHttpHelperTest, ToWideString_Empty) {
    EXPECT_TRUE(toWideString("").empty());
}

TEST(WinHttpHelperTest, ToWideString_Ascii) {
    auto ws = toWideString("hello");
    EXPECT_EQ(ws, L"hello");
}

TEST(WinHttpHelperTest, ToWideString_Utf8) {
    std::string utf8 = "\xe4\xbd\xa0\xe5\xa5\xbd";
    auto ws = toWideString(utf8);
    EXPECT_EQ(ws.size(), 2u);
    EXPECT_EQ(ws[0], static_cast<wchar_t>(0x4F60));
    EXPECT_EQ(ws[1], static_cast<wchar_t>(0x597D));
}

TEST(WinHttpHelperTest, ToWideString_Url) {
    auto ws = toWideString("https://oss-cn-hangzhou.aliyuncs.com/bucket/key?q=1&b=2");
    EXPECT_EQ(ws, L"https://oss-cn-hangzhou.aliyuncs.com/bucket/key?q=1&b=2");
}

// ---------- fromWideString ----------

TEST(WinHttpHelperTest, FromWideString_Empty) {
    EXPECT_TRUE(fromWideString(L"").empty());
}

TEST(WinHttpHelperTest, FromWideString_Ascii) {
    auto s = fromWideString(L"world");
    EXPECT_EQ(s, "world");
}

TEST(WinHttpHelperTest, FromWideString_Utf8) {
    std::wstring ws = {static_cast<wchar_t>(0x4F60), static_cast<wchar_t>(0x597D)};
    auto s = fromWideString(ws);
    EXPECT_EQ(s, "\xe4\xbd\xa0\xe5\xa5\xbd");
}

TEST(WinHttpHelperTest, RoundTrip) {
    std::string original = "https://bucket.oss-cn-shanghai.aliyuncs.com/object";
    EXPECT_EQ(fromWideString(toWideString(original)), original);
}

// ---------- makeWinHttpError ----------

TEST(WinHttpHelperTest, MakeWinHttpError_ConnectionFailed) {
    auto err = makeWinHttpError(TransportErrorCode::ConnectionFailed, 12029);
    EXPECT_EQ(err.error, make_error_code(TransportErrorCode::ConnectionFailed));
    EXPECT_EQ(err.errorCode, "WinHttpError");
    EXPECT_TRUE(err.errorMessage.find("12029") != std::string::npos);
}

TEST(WinHttpHelperTest, MakeWinHttpError_Timeout) {
    auto err = makeWinHttpError(TransportErrorCode::Timeout, 12002);
    EXPECT_EQ(err.error, make_error_code(TransportErrorCode::Timeout));
    EXPECT_EQ(err.errorCode, "WinHttpError");
    EXPECT_TRUE(err.errorMessage.find("12002") != std::string::npos);
}

TEST(WinHttpHelperTest, MakeWinHttpError_SendRecvError) {
    auto err = makeWinHttpError(TransportErrorCode::SendRecvError, 12030);
    EXPECT_EQ(err.error, make_error_code(TransportErrorCode::SendRecvError));
}

// ---------- buildConnectionOptions(HttpTransportOptions) ----------

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_Defaults) {
    HttpTransportOptions opts;
    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.verifySSL);
    EXPECT_FALSE(conn.enabledRedirect);
    EXPECT_TRUE(conn.proxyHost.empty());
    EXPECT_EQ(conn.proxyPort, 0u);
}

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_InsecureSkipVerify) {
    HttpTransportOptions opts;
    opts.insecureSkipVerify = true;
    auto conn = buildConnectionOptions(opts);
    EXPECT_FALSE(conn.verifySSL);
}

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_EnabledRedirect) {
    HttpTransportOptions opts;
    opts.enabledRedirect = true;
    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.enabledRedirect);
}

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_ProxyHost) {
    HttpTransportOptions opts;
    opts.proxyHost = "http://proxy.example.com";
    auto conn = buildConnectionOptions(opts);
    EXPECT_EQ(conn.proxyHost, "http://proxy.example.com");
}

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_EmptyProxyHost) {
    HttpTransportOptions opts;
    opts.proxyHost = "";
    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.proxyHost.empty());
}

// ---------- buildConnectionOptions(WinHttpTransportOptions) ----------

TEST(WinHttpHelperTest, BuildConnOpts_WinHttpTransportOptions_AllFields) {
    WinHttpTransportOptions opts;
    opts.insecureSkipVerify = false;
    opts.enabledRedirect = true;
    opts.proxyHost = "http://proxy.local";
    opts.proxyPort = 8080u;
    opts.proxyUserName = "user";
    opts.proxyPassword = "pass";

    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.verifySSL);
    EXPECT_TRUE(conn.enabledRedirect);
    EXPECT_EQ(conn.proxyHost, "http://proxy.local");
    EXPECT_EQ(conn.proxyPort, 8080u);
    EXPECT_EQ(conn.proxyUserName, "user");
    EXPECT_EQ(conn.proxyPassword, "pass");
}

TEST(WinHttpHelperTest, BuildConnOpts_WinHttpTransportOptions_InheritsBase) {
    WinHttpTransportOptions opts;
    opts.insecureSkipVerify = true;
    opts.proxyHost = "http://base-proxy";

    auto conn = buildConnectionOptions(opts);
    EXPECT_FALSE(conn.verifySSL);
    EXPECT_EQ(conn.proxyHost, "http://base-proxy");
}

TEST(WinHttpHelperTest, BuildConnOpts_WinHttpTransportOptions_PartialFields) {
    WinHttpTransportOptions opts;
    opts.proxyPort = 3128u;

    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.verifySSL);
    EXPECT_TRUE(conn.proxyHost.empty());
    EXPECT_EQ(conn.proxyPort, 3128u);
    EXPECT_TRUE(conn.proxyUserName.empty());
    EXPECT_TRUE(conn.proxyPassword.empty());
}

// ---------- WinHttpTransportFactory ----------

TEST(WinHttpTransportFactoryTest, CreateHttpTransport) {
    WinHttpTransportOptions opts;
    auto transport = WinHttpTransportFactory::createHttpTransport(opts);
    ASSERT_NE(transport, nullptr);
    EXPECT_EQ(transport->getName(), "winhttp");
}

// ---------- parseResponseHeaders ----------

TEST(WinHttpHelperTest, ParseResponseHeaders_Normal) {
    std::string raw = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 1234\r\n\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["Content-Type"], "text/html");
    EXPECT_EQ(headers["Content-Length"], "1234");
    EXPECT_EQ(headers.count("HTTP/1.1 200 OK"), 0u);
}

TEST(WinHttpHelperTest, ParseResponseHeaders_NoSpaceAfterColon) {
    std::string raw = "X-Custom:value-no-space\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["X-Custom"], "value-no-space");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_MultipleSpaces) {
    std::string raw = "X-Custom:   lots-of-space   \r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["X-Custom"], "lots-of-space");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_TabWhitespace) {
    std::string raw = "X-Custom:\tvalue\t\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["X-Custom"], "value");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_EmptyValue) {
    std::string raw = "X-Empty:\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["X-Empty"], "");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_ColonInValue) {
    std::string raw = "Location: http://example.com:8080/path\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers["Location"], "http://example.com:8080/path");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_SkipsStatusLine) {
    std::string raw = "HTTP/1.1 200 OK\r\nServer: nginx\r\n\r\n";
    auto headers = parseResponseHeaders(raw);
    EXPECT_EQ(headers.count("HTTP/1.1 200 OK"), 0u);
    EXPECT_EQ(headers["Server"], "nginx");
}

TEST(WinHttpHelperTest, ParseResponseHeaders_Empty) {
    auto headers = parseResponseHeaders("");
    EXPECT_TRUE(headers.empty());
}

// ---------- WinHttpTransportFactory (async) ----------

TEST(WinHttpTransportFactoryTest, CreateAsyncHttpTransport) {
    WinHttpTransportOptions opts;
    auto transport = WinHttpTransportFactory::createAsyncHttpTransport(opts);
    ASSERT_NE(transport, nullptr);
    EXPECT_EQ(transport->getName(), "winhttp-async");
}

// ---------- Default constants ----------

TEST(WinHttpHelperTest, DefaultConstants) {
    EXPECT_EQ(kDefaultConnectTimeoutMs, 5000);
    EXPECT_EQ(kDefaultReadWriteTimeoutMs, 10000);
    EXPECT_EQ(kDefaultMaxConnectionsSync, 16u);
    EXPECT_EQ(kDefaultMaxConnectionsAsync, 100u);
    EXPECT_EQ(kWriteBufferLength, 64u * 1024u);
}

// ========== makeHttpMetrics ==========

TEST(WinHttpHelperTest, MakeHttpMetrics_Enabled) {
    auto metrics = makeHttpMetrics(true);
    ASSERT_NE(metrics, nullptr);
    EXPECT_EQ(metrics->dnsLookup.count(), 0);
    EXPECT_EQ(metrics->connect.count(), 0);
    EXPECT_EQ(metrics->tlsHandshake.count(), 0);
    EXPECT_EQ(metrics->startTransfer.count(), 0);
    EXPECT_EQ(metrics->total.count(), 0);
    EXPECT_FALSE(metrics->connectionReused);
}

TEST(WinHttpHelperTest, MakeHttpMetrics_Disabled) {
    auto metrics = makeHttpMetrics(false);
    EXPECT_EQ(metrics, nullptr);
}

// ========== beforeRequestMetrics ==========

TEST(WinHttpHelperTest, BeforeRequestMetrics_SetsRequestStart) {
    auto metrics = makeHttpMetrics(true);
    auto before = std::chrono::system_clock::now();
    beforeRequestMetrics(metrics.get());
    auto after = std::chrono::system_clock::now();
    EXPECT_GE(metrics->requestStart, before);
    EXPECT_LE(metrics->requestStart, after);
}

TEST(WinHttpHelperTest, BeforeRequestMetrics_NullIsNoop) {
    beforeRequestMetrics(nullptr);
}

// ========== afterRequestMetrics ==========

TEST(WinHttpHelperTest, AfterRequestMetrics_NullMetricsIsNoop) {
    afterRequestMetrics(nullptr, nullptr);
}

TEST(WinHttpHelperTest, AfterRequestMetrics_NullHandleIsNoop) {
    auto metrics = makeHttpMetrics(true);
    afterRequestMetrics(metrics.get(), nullptr);
    EXPECT_EQ(metrics->total.count(), 0);
}

// ========== buildConnectionOptions collectMetrics ==========

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_CollectMetricsDefault) {
    HttpTransportOptions opts;
    auto conn = buildConnectionOptions(opts);
    EXPECT_FALSE(conn.collectMetrics);
}

TEST(WinHttpHelperTest, BuildConnOpts_HttpTransportOptions_CollectMetricsEnabled) {
    HttpTransportOptions opts;
    opts.collectMetrics = true;
    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.collectMetrics);
}

TEST(WinHttpHelperTest, BuildConnOpts_WinHttpTransportOptions_CollectMetrics) {
    WinHttpTransportOptions opts;
    opts.collectMetrics = true;
    auto conn = buildConnectionOptions(opts);
    EXPECT_TRUE(conn.collectMetrics);
}

} // namespace alibabacloud::oss2::transport::winhttp
