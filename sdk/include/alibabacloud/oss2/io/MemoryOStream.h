#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <cstddef>
#include <iostream>
#include <streambuf>

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API MemoryStreamBuf : public std::streambuf {
  public:
    MemoryStreamBuf(char* buf, std::size_t size) { setp(buf, buf + size); }
    std::size_t written() const { return static_cast<std::size_t>(pptr() - pbase()); }
};

class ALIBABACLOUD_OSS_API MemoryOStream : public std::ostream {
  public:
    MemoryOStream(char* buf, std::size_t size) : std::ostream(&sbuf_), sbuf_(buf, size) {}
    std::size_t written() const { return sbuf_.written(); }

  private:
    MemoryStreamBuf sbuf_;
};

} // namespace oss2
} // namespace alibabacloud
