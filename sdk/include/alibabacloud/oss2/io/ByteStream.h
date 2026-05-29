#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>


namespace alibabacloud {
namespace oss2 {


/**
 * @brief A single-pass, sequential read cursor over a byte sequence.
 *
 * ByteSource is the read-side counterpart of ByteContent. Each call to
 * ByteContent::spanSource() produces a fresh ByteSource that reads from
 * the beginning. A ByteSource is consumed at most once and should not be
 * shared across threads.
 *
 * State flags follow the same semantics as std::istream::rdstate():
 *   - goodbit (0): no errors
 *   - eofbit  (1): end-of-data reached during a read
 *   - failbit (2): a read could not satisfy the requested count
 *   - badbit  (4): an unrecoverable I/O error occurred
 */
class ALIBABACLOUD_OSS_API ByteSource {
  private:
    /**
     * @brief Reads up to @p count bytes into @p buffer.
     * @return The number of bytes actually read.
     */
    virtual std::size_t onRead(std::uint8_t* buffer, std::size_t count) = 0;

    /**
     * @brief Returns the current error-state flags (see state()).
     */
    virtual int iostate() = 0;

  public:
    virtual ~ByteSource() = default;

    /**
     * @brief Reads up to @p count bytes into @p buffer.
     * @return The number of bytes actually read. A return value less than
     *         @p count indicates end-of-data or an error (check state()).
     */
    std::size_t read(std::uint8_t* buffer, std::size_t count) {
        return onRead(buffer, count);
    }

    /**
     * @brief Reads exactly @p count bytes, or until end-of-data is reached.
     *
     * Internally loops over read() until either @p count bytes have been
     * collected or read() returns 0.
     * @return The total number of bytes read (may be less than @p count).
     */
    std::size_t readToCount(std::uint8_t* buffer, std::size_t count);

    /**
     * @brief Reads all remaining data and returns it as a byte vector.
     *
     * Allocates memory in 8 KB chunks. Intended for small payloads such as
     * XML response bodies; do not use for large file downloads.
     */
    std::vector<std::uint8_t> readToEnd();

    /**
     * @brief Returns the current stream error-state flags.
     *
     * Bitmask values are identical to std::ios_base::iostate:
     *   - good bit 0x0  No errors (zero value)
     *   - eof  bit 0x1  End-of-data reached on read
     *   - fail bit 0x2  Logical error on I/O operation
     *   - bad  bit 0x4  Unrecoverable read error
     */
    int state() {
        return iostate();
    }
};


/**
 * @brief Abstract base for request body content.
 *
 * ByteContent represents a replayable (or one-shot) data source that can
 * produce one or more ByteSource cursors via spanSource(). The SDK calls
 * spanSource() once per HTTP attempt; on retries a new cursor is created
 * so that the data can be re-read from the beginning.
 *
 * Concrete implementations:
 *   - StringContent   : owns a std::string
 *   - StreamContent   : references a shared_ptr<std::istream>
 *   - FileContent     : owns a file path, opens a new handle per span
 *   - MemoryContent   : non-owning (borrows a string_view, zero-copy)
 *   - EmptyContent    : zero-length body
 *
 * Typical usage through the RequestBody helpers:
 * @code
 *   // From an owned string
 *   auto body = RequestBody::fromString("hello");
 *
 *   // From a file (supports offset + length for multipart upload)
 *   auto body = RequestBody::fromFile("/path/to/data.bin");
 *
 *   // From a std::istream
 *   auto ifs = std::make_shared<std::ifstream>("data.bin", std::ios::binary);
 *   auto body = RequestBody::fromStream(ifs);
 *
 *   // From externally managed memory (zero-copy, caller must keep data alive)
 *   auto body = RequestBody::fromMemory(ptr, len);
 * @endcode
 */
class ALIBABACLOUD_OSS_API ByteContent {
  public:
    virtual ~ByteContent() = default;

    /**
     * @brief Returns the content length in bytes, or std::nullopt if unknown.
     *
     * When the length is known, the SDK sets the Content-Length header
     * automatically. When unknown, chunked transfer encoding may be used.
     */
    virtual std::optional<std::size_t> length() const = 0;

    /**
     * @brief Indicates whether the content can only be consumed once.
     *
     * If true, the SDK will not retry the request on transient failures
     * because the underlying data cannot be re-read. Non-seekable streams
     * (e.g., pipes, network sockets) should return true.
     */
    virtual bool isOneShot() const = 0;

    /**
     * @brief Creates a new read cursor over this content.
     *
     * Each call returns an independent ByteSource that starts reading from
     * the beginning. The returned ByteSource does not own the underlying
     * data; callers must ensure that this ByteContent outlives the
     * ByteSource.
     */
    virtual std::unique_ptr<ByteSource> spanSource() = 0;

    /**
     * @brief Returns the filesystem path of the content, if applicable.
     *
     * Only FileContent returns a valid path. This is reserved for transport
     * layer optimizations (e.g., sendfile() system call). The default
     * implementation returns std::nullopt.
     */
    virtual std::optional<std::filesystem::path> path() const {
        return std::nullopt;
    };
};

/**
 * @brief Owning content backed by a std::string.
 *
 * The string is copied or moved into this object and is owned for the
 * lifetime of the StringContent instance. Always replayable.
 *
 * @code
 *   auto body = std::make_shared<StringContent>("<?xml ...>");
 *   // or use the helper:
 *   auto body = RequestBody::fromString("<?xml ...>");
 * @endcode
 */
class ALIBABACLOUD_OSS_API StringContent : public ByteContent {
  public:
    StringContent(const std::string& content) : content_(content) {}
    StringContent(std::string&& content) : content_(std::move(content)) {}
    std::optional<std::size_t> length() const override {
        return content_.size();
    }
    bool isOneShot() const override {
        return false;
    }
    std::unique_ptr<ByteSource> spanSource() override;

  private:
    std::string content_;
};

/**
 * @brief Content backed by a shared std::istream.
 *
 * StreamContent references (but does not own exclusively) an istream via
 * shared_ptr. On construction it probes whether the stream is seekable;
 * if so, spanSource() can rewind the stream for retries (isOneShot() == false).
 *
 * Two constructors are available:
 *   - Auto-detect: probes seekability and computes length by seeking.
 *   - Explicit:    caller supplies seekable flag and optional length,
 *                  avoiding seek side-effects on non-seekable streams.
 *
 * @code
 *   // Auto-detect (suitable for file-backed streams)
 *   auto ifs = std::make_shared<std::ifstream>("data.bin", std::ios::binary);
 *   auto body = std::make_shared<StreamContent>(ifs);
 *
 *   // Explicit (suitable for pipes or network streams)
 *   auto body = std::make_shared<StreamContent>(pipeStream, false);
 *
 *   // Or use the helper:
 *   auto body = RequestBody::fromStream(ifs);
 * @endcode
 */
class ALIBABACLOUD_OSS_API StreamContent : public ByteContent {
  public:
    StreamContent(std::shared_ptr<std::istream> content);
    StreamContent(std::shared_ptr<std::istream> content, bool seekable,
                  std::optional<std::size_t> length = std::nullopt);
    std::optional<std::size_t> length() const override {
        return length_;
    }
    bool isOneShot() const override {
        return !seekable_;
    }
    std::unique_ptr<ByteSource> spanSource() override;

  protected:
    std::shared_ptr<std::istream> content_;
    std::optional<std::size_t> length_;
    std::streampos pos_;
    bool seekable_;
    bool spanned_;
};

/**
 * @brief Owning content backed by a filesystem path.
 *
 * Each call to spanSource() opens a new file handle positioned at the
 * given offset, making FileContent inherently safe for retries and
 * concurrent reads. The optional @p length parameter limits how many
 * bytes the transport layer will read, which is useful for multipart
 * upload parts.
 *
 * @code
 *   // Upload an entire file
 *   auto body = std::make_shared<FileContent>("/path/to/data.bin");
 *
 *   // Upload bytes [1048576, 2097152) as one multipart part
 *   auto body = std::make_shared<FileContent>("/path/to/data.bin", 1048576, 1048576);
 *
 *   // Or use the helper:
 *   auto body = RequestBody::fromFile("/path/to/data.bin");
 * @endcode
 */
class ALIBABACLOUD_OSS_API FileContent : public ByteContent {
  public:
    FileContent(std::string path, std::size_t off = 0, std::optional<std::size_t> length = std::nullopt)
        : FileContent(std::filesystem::path(std::move(path)), off, length) {}

    FileContent(std::filesystem::path path, std::size_t off = 0, std::optional<std::size_t> length = std::nullopt);

    std::optional<std::size_t> length() const override {
        return length_;
    }
    bool isOneShot() const override {
        return false;
    }
    std::unique_ptr<ByteSource> spanSource() override;
    std::optional<std::filesystem::path> path() const override {
        return path_;
    }

  protected:
    std::filesystem::path path_;
    std::size_t off_;
    std::optional<std::size_t> length_;
};

/**
 * @brief Non-owning content that borrows externally managed memory (zero-copy).
 *
 * MemoryContent holds a std::string_view and does NOT copy or own the
 * underlying data. This is intentional: it enables zero-copy request bodies
 * when the caller can guarantee that the pointed-to memory remains valid
 * for the entire lifetime of the MemoryContent and any ByteSource produced
 * from it.
 *
 * @warning The caller MUST ensure the referenced memory outlives this object
 *          and all ByteSource instances created via spanSource(). Passing a
 *          temporary std::string or a buffer that is freed before the HTTP
 *          request completes will result in undefined behavior.
 *
 * @code
 *   // CORRECT: data is a local variable whose lifetime covers the request
 *   std::string xml = buildXmlPayload();
 *   auto body = std::make_shared<MemoryContent>(xml);
 *   client.putObject(request.setBody(body));
 *   // 'xml' is still alive here; safe.
 *
 *   // CORRECT: static / global data
 *   static const char kPayload[] = "...";
 *   auto body = RequestBody::fromMemory(kPayload, sizeof(kPayload) - 1);
 *
 *   // WRONG: temporary string destroyed immediately
 *   auto body = std::make_shared<MemoryContent>(std::string("hello"));
 *   // The temporary is gone -- body->spanSource() reads garbage!
 *
 *   // If you need owning semantics, use StringContent instead:
 *   auto body = RequestBody::fromString("hello");  // safe, data is copied
 * @endcode
 */
class ALIBABACLOUD_OSS_API MemoryContent : public ByteContent {
  public:
    MemoryContent(std::string_view content) : content_(std::move(content)) {}
    MemoryContent(const char* content, size_t len) : content_(std::string_view(content, len)) {}
    std::optional<std::size_t> length() const override {
        return content_.size();
    }
    bool isOneShot() const override {
        return false;
    }
    std::unique_ptr<ByteSource> spanSource() override;

  protected:
    std::string_view content_;
};

/**
 * @brief A zero-length content representing an empty request body.
 *
 * Used for requests that require a body field but carry no payload
 * (e.g., DELETE, HEAD when constructed via RequestBody helpers).
 */
class ALIBABACLOUD_OSS_API EmptyContent : public ByteContent {
  public:
    EmptyContent() = default;
    std::optional<std::size_t> length() const override {
        return 0;
    }
    bool isOneShot() const override {
        return false;
    }
    std::unique_ptr<ByteSource> spanSource() override;
};


/**
 * @brief Factory for creating output streams to receive response data.
 *
 * OStreamSupplier provides the write-side abstraction for response bodies.
 * The transport layer calls getOStream() to obtain a stream to write
 * received data into.
 *
 * When isOneShot() returns false, the supplier can be called multiple times
 * (e.g., on retries) to create fresh output streams.
 */
class ALIBABACLOUD_OSS_API OStreamSupplier {
  public:
    virtual ~OStreamSupplier() = default;

    /**
     * @brief Indicates whether the supplier can only produce one stream.
     */
    virtual bool isOneShot() const = 0;

    /**
     * @brief Creates or returns an output stream to write response data into.
     */
    virtual std::shared_ptr<std::ostream> getOStream() = 0;

  public:
    /**
     * @brief Convenience factory that wraps a callable into an OStreamSupplier.
     * @param supplier  A callable that returns a shared_ptr<std::ostream>.
     * @param reuse     If true, the supplier may be called multiple times (retries).
     */
    static std::unique_ptr<OStreamSupplier> from(std::function<std::shared_ptr<std::ostream>()> supplier, bool reuse);
};

} // namespace oss2
} // namespace alibabacloud