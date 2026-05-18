#pragma once

#include <cstdint>

namespace alibabacloud::oss2::transport {

// Timeout defaults
constexpr long kDefaultConnectTimeoutMs = 5000;        // 5s
constexpr long kDefaultReadWriteTimeoutMs = 10000;     // 10s

// Max concurrent connections per host
constexpr unsigned int kDefaultMaxConnectionsSync = 16;
constexpr unsigned int kDefaultMaxConnectionsAsync = 100;

// I/O buffer size for request/response body transfer
constexpr uint32_t kWriteBufferLength = 64 * 1024;

} // namespace alibabacloud::oss2::transport
