#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

#include <cstring>

namespace alibabacloud {
namespace oss2 {


OStreamWriter::OStreamWriter(std::shared_ptr<std::ostream> os) : os_(std::move(os)) {}

std::size_t OStreamWriter::onWrite(const std::uint8_t* data, std::size_t n) {
    os_->write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(n));
    return os_->fail() ? 0 : n;
}

int OStreamWriter::iostate() const {
    return os_->rdstate();
}


std::size_t ObservableWriter::onWrite(const std::uint8_t* data, std::size_t n) {
    auto written = writer_->write(data, n);
    if (written > 0) {
        for (auto& o : observers_) {
            o->write(data, written);
        }
    }
    return written;
}

int ObservableWriter::iostate() const {
    return writer_->state();
}

ProgressWriteObserver::ProgressWriteObserver(ProgressCallback callback, std::int64_t total)
    : callback_(std::move(callback)), total_(total) {}

std::size_t ProgressWriteObserver::onWrite(const std::uint8_t*, std::size_t n) {
    transferred_ += n;
    if (transferred_ > lastTransferred_) {
        callback_(n, transferred_, total_);
    }
    return n;
}

void ProgressWriteObserver::onReset() {
    lastTransferred_ = transferred_;
    transferred_ = 0;
}


std::size_t CRC64WriteObserver::onWrite(const std::uint8_t* data, std::size_t n) {
    value_ = utils::CalcCRC64(value_, data, n);
    return n;
}

void CRC64WriteObserver::onReset() {
    value_ = init_;
}


MemoryWriter::MemoryWriter(std::uint8_t* buf, std::size_t capacity) : buf_(buf), capacity_(capacity) {}

std::size_t MemoryWriter::onWrite(const std::uint8_t* data, std::size_t n) {
    if (offset_ + n > capacity_) {
        overflow_ = true;
        return 0;
    }
    std::memcpy(buf_ + offset_, data, n);
    offset_ += n;
    return n;
}

int MemoryWriter::iostate() const {
    return overflow_ ? std::ios_base::failbit : 0;
}

} // namespace oss2
} // namespace alibabacloud
