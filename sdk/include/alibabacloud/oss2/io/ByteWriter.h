#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include "alibabacloud/oss2/Types.h"

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <ostream>
#include <vector>

namespace alibabacloud {
namespace oss2 {


/**
 * @brief A single-pass, sequential write sink for byte data.
 *
 * ByteWriter is the write-side counterpart of ByteSource. The transport
 * layer calls write() to push received response data into the sink.
 * A ByteWriter is consumed at most once per HTTP attempt; on retries a
 * new ByteWriter is obtained from SinkFactory.
 *
 * State flags follow the same semantics as std::ostream::rdstate():
 *   - goodbit (0): no errors
 *   - failbit (2): a logical I/O error occurred
 *   - badbit  (4): an unrecoverable I/O error occurred
 */
class ALIBABACLOUD_OSS_API ByteWriter {
  private:
    /**
     * @brief Writes @p n bytes from @p data into the sink.
     * @return The number of bytes actually written.
     */
    virtual std::size_t onWrite(const std::uint8_t* data, std::size_t n) = 0;

    /**
     * @brief Returns the current error-state flags (see state()).
     */
    virtual int iostate() const = 0;

  public:
    virtual ~ByteWriter() = default;

    /**
     * @brief Writes @p n bytes from @p data into the sink.
     * @return The number of bytes actually written. A return value of 0
     *         indicates an error (check state()).
     */
    std::size_t write(const std::uint8_t* data, std::size_t n) { return onWrite(data, n); }

    /**
     * @brief Returns the current stream error-state flags.
     *
     * Bitmask values are identical to std::ios_base::iostate:
     *   - good bit 0x0  No errors (zero value)
     *   - fail bit 0x2  Logical error on I/O operation
     *   - bad  bit 0x4  Unrecoverable write error
     */
    int state() const { return iostate(); }

    /// Returns true if no error flags are set.
    bool good() const { return iostate() == std::ios_base::goodbit; }

    /// Returns true if failbit or badbit is set.
    bool fail() const { return (iostate() & (std::ios_base::badbit | std::ios_base::failbit)) != 0; }

    /// Returns true if badbit is set (unrecoverable I/O error).
    bool bad() const { return (iostate() & std::ios_base::badbit) != 0; }
};


/**
 * @brief Adapter that bridges ByteWriter to a std::ostream.
 *
 * OStreamWriter wraps a shared_ptr<std::ostream> and delegates write()
 * calls to ostream::write(). The iostate() is proxied directly from the
 * underlying ostream's rdstate(), so good()/fail()/bad() behave
 * identically to the wrapped stream.
 *
 * @code
 *   auto file = std::make_shared<std::ofstream>("out.dat", std::ios::binary);
 *   auto writer = std::make_shared<OStreamWriter>(file);
 * @endcode
 */
class ALIBABACLOUD_OSS_API OStreamWriter : public ByteWriter {
  public:
    explicit OStreamWriter(std::shared_ptr<std::ostream> os);

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    int iostate() const override;

    std::shared_ptr<std::ostream> os_;
};


/**
 * @brief Abstract base for observers attached to an ObservableWriter.
 *
 * ByteWriterObserver extends ByteWriter with a reset() interface. When
 * the transport layer retries a request, ObservableWriter::reset() is
 * called, which in turn calls onReset() on each attached observer so
 * they can clear accumulated state (e.g., progress counters, CRC values).
 *
 * Observers always report iostate() == 0 (good) because they do not
 * perform real I/O -- they only observe the data flowing through.
 */
class ALIBABACLOUD_OSS_API ByteWriterObserver : public ByteWriter {
  private:
    /**
     * @brief Resets the observer's internal state for a retry.
     */
    virtual void onReset() = 0;

    int iostate() const override { return 0; }

  public:
    /// Resets the observer's accumulated state (e.g., for retries).
    void reset() { onReset(); }
};


/**
 * @brief A ByteWriter that forwards data to a primary writer and observers.
 *
 * ObservableWriter delegates write() to the primary writer_ and also
 * fans out the same data to all attached ByteWriterObserver instances.
 * The state is determined solely by the primary writer -- observers
 * do not affect error reporting.
 *
 * @code
 *   auto file = std::make_shared<OStreamWriter>(
 *       std::make_shared<std::ofstream>("local.dat", std::ios::binary));
 *   auto progress = std::make_shared<ProgressWriteObserver>(cb, contentLength);
 *   auto crc = std::make_shared<CRC64WriteObserver>();
 *
 *   auto sink = std::make_shared<ObservableWriter>(file, progress, crc);
 *   // sink->write() writes to file, updates progress, and computes CRC
 * @endcode
 */
class ALIBABACLOUD_OSS_API ObservableWriter : public ByteWriter {
  public:
    template <typename... Observers>
    ObservableWriter(std::shared_ptr<ByteWriter> writer, std::shared_ptr<Observers>... observers)
        : writer_(std::move(writer)) {
        (observers_.push_back(std::move(observers)), ...);
    }

    void addObserver(std::shared_ptr<ByteWriterObserver> observer) {
        observers_.push_back(std::move(observer));
    }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    int iostate() const override;

    std::shared_ptr<ByteWriter> writer_;
    std::vector<std::shared_ptr<ByteWriterObserver>> observers_;
};


/**
 * @brief Observer that tracks download progress via a callback.
 *
 * Each write() invokes the ProgressCallback with the incremental byte
 * count, total bytes transferred so far, and the expected total size.
 *
 * On reset(), the high-water mark is saved. Subsequent writes suppress
 * the callback until the transferred count exceeds the previous mark,
 * preventing duplicate progress reports during retries.
 */
class ALIBABACLOUD_OSS_API ProgressWriteObserver : public ByteWriterObserver {
  public:
    ProgressWriteObserver(ProgressCallback callback, std::int64_t total);

    void updateTotal(std::int64_t total) { total_ = total; }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    void onReset() override;

    ProgressCallback callback_;
    std::int64_t total_;
    std::size_t transferred_{0};
    std::size_t lastTransferred_{0};
};


/**
 * @brief Observer that computes a running CRC-64 checksum over written data.
 *
 * The CRC value is accumulated incrementally with each write() call.
 * Call crc() to retrieve the current checksum.
 *
 * On reset(), the CRC is restored to the initial value (default 0),
 * allowing a clean recomputation on retries.
 *
 * @code
 *   auto crc = std::make_shared<CRC64WriteObserver>();
 *   // ... attach to ObservableWriter and perform download ...
 *   uint64_t checksum = crc->crc();
 * @endcode
 */
class ALIBABACLOUD_OSS_API CRC64WriteObserver : public ByteWriterObserver {
  public:
    explicit CRC64WriteObserver(uint64_t init = 0) : init_(init), value_(init) {}

    /// Returns the current CRC-64 checksum value.
    inline uint64_t crc() const { return value_; }

    /// Returns the current CRC-64 checksum as a decimal string.
    inline std::string crcAsString() const { return std::to_string(value_); }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    void onReset() override;

    uint64_t init_;
    uint64_t value_;
};


/**
 * @brief A ByteWriter that writes directly into a user-provided memory buffer.
 *
 * MemoryWriter is a zero-copy alternative to OStreamWriter when the caller
 * already has a contiguous buffer. It writes sequentially until the buffer
 * is full; any attempt to write beyond capacity sets failbit and writes
 * nothing for that call.
 *
 * @code
 *   std::uint8_t buf[4096];
 *   auto writer = std::make_shared<MemoryWriter>(buf, sizeof(buf));
 *   // ... use as sink in SinkFactory ...
 *   std::size_t received = writer->written();
 * @endcode
 */
class ALIBABACLOUD_OSS_API MemoryWriter : public ByteWriter {
  public:
    MemoryWriter(std::uint8_t* buf, std::size_t capacity);

    /// Returns the number of bytes written so far.
    std::size_t written() const { return offset_; }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    int iostate() const override;

    std::uint8_t* buf_;
    std::size_t capacity_;
    std::size_t offset_{0};
    bool overflow_{false};
};

} // namespace oss2
} // namespace alibabacloud
