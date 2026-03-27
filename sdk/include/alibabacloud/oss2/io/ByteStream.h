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


class ALIBABACLOUD_OSS_API ByteSource {
  private:
    /**
     * Read portion of data into a buffer.
     */
    virtual std::size_t onRead(std::uint8_t* buffer, std::size_t count) = 0;

    /**
     * Type for source state flags
     */
    virtual int iostate() = 0;

  public:
    virtual ~ByteSource() = default;

    /**
     * Read portion of data into a buffer.
     */
    std::size_t read(std::uint8_t* buffer, std::size_t count) {
        return onRead(buffer, count);
    }

    /**
     * Read into a buffer until the buffer is filled, or until the stream is read to end.
     */
    std::size_t readToCount(std::uint8_t* buffer, std::size_t count);

    /**
     * Read until the stream is read to end, allocating memory
     * for the entirety of contents.
     */
    std::vector<std::uint8_t> readToEnd();

    /**
     * Bitmask type to represent stream error state flags.
     * This flag is set by operations performed on the stream
     * when an error occurs while read, generally causing the loss of integrity of the stream
     * The same as std::ios_base::rdstate
     * good bit 0x0 No errors (zero value iostate)
     * eof  bit 0x1 End-of-File reached on input operation
     * fail bit 0x2 Logical error on i/o operation
     * bad  bit 0x4 Read error on i/o operation
     */
    int state() {
        return iostate();
    }
};


class ALIBABACLOUD_OSS_API ByteContent {
  public:
    virtual ~ByteContent() = default;

    /**
     * The content length if known
     */
    virtual std::optional<std::size_t> length() const = 0;

    /**
     * Flag indicating if the body can only be consumed once. If false the underlying stream
     * must be capable of being replayed.
     */
    virtual bool isOneShot() const = 0;

    /**
     * Provides non-owning ByteSource to read from/consume
     */
    virtual std::unique_ptr<ByteSource> spanSource() = 0;

    /**
     * The path of content if known
     */
    virtual std::optional<std::filesystem::path> path() const {
        return std::nullopt;
    };
};

/**
 * Container for wrapping a string as a ByteContent
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
 * Container for wrapping a istream as a ByteContent
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
 * Container for wrapping a file as a ByteContent
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
 * Container for wrapping a memeory data as a ByteContent
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
 * Container for wrapping a null data as a ByteContent
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


class ALIBABACLOUD_OSS_API OStreamSupplier {
  public:
    virtual ~OStreamSupplier() = default;

    /**
     * Flag indicating if the writer can only be use once.
     */
    virtual bool isOneShot() const = 0;

    /**
     * Provides std::ostream to write to
     */
    virtual std::shared_ptr<std::ostream> getOStream() = 0;

  public:
    static std::unique_ptr<OStreamSupplier> from(std::function<std::shared_ptr<std::ostream>()> supplier, bool reuse);
};

} // namespace oss2
} // namespace alibabacloud