
#include "alibabacloud/oss2/io/ByteStream.h"

#include <cstring>
#include <filesystem>
#include <fstream>

namespace alibabacloud {
namespace oss2 {

namespace detail {


class MemoryByteSource final : public ByteSource {
  private:
    const std::uint8_t* data_;
    std::size_t length_;
    std::size_t offset_ = 0;
    int state = 0;

    std::size_t onRead(std::uint8_t* buffer, std::size_t count) override {
        if (this->offset_ + count > this->length_) {
            state = std::ios::eofbit + std::ios::failbit;
        }
        size_t copy_length = (std::min)(count, this->length_ - this->offset_);
        std::memcpy(buffer, this->data_ + offset_, static_cast<size_t>(copy_length));
        offset_ += copy_length;
        return copy_length;
    }

    int iostate() override {
        return state;
    }

  public:
    MemoryByteSource(const uint8_t* data, size_t length) : data_(data), length_(length) {}
};

class StreamSource final : public ByteSource {
  private:
    std::unique_ptr<std::istream> is_;
    std::istream* isPtr_;
    std::size_t onRead(std::uint8_t* buffer, std::size_t count) override {
        if (isPtr_ && *isPtr_) {
            isPtr_->read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(count));
            return static_cast<size_t>(isPtr_->gcount());
        } else {
            return 0;
        }
    }

    int iostate() override {
        return isPtr_ ? isPtr_->rdstate() : std::ios::badbit;
    }

  public:
    StreamSource(std::unique_ptr<std::istream> is) : is_(std::move(is)), isPtr_(nullptr) {
        if (is_ != nullptr) {
            isPtr_ = is_.get();
        }
    }
    StreamSource(std::istream* isPtr) : is_(nullptr), isPtr_(isPtr) {}
};


class OStreamSupplierImpl : public OStreamSupplier {
  public:
    explicit OStreamSupplierImpl(std::function<std::shared_ptr<std::ostream>()> supplier, bool reuse)
            : supplier_(std::move(supplier)), reuse_(reuse) {}

    bool isOneShot() const override {
        return !reuse_;
    }

    /**
     * Provides std::ostream to write to
     */
    std::shared_ptr<std::ostream> getOStream() override {
        if (supplier_ != nullptr) {
            return supplier_();
        }
        return nullptr;
    }

  private:
    std::function<std::shared_ptr<std::ostream>()> supplier_;
    bool reuse_;
};

} // namespace detail

std::size_t ByteSource::readToCount(std::uint8_t* buffer, std::size_t count) {
    size_t totalRead = 0;
    for (;;) {
        size_t readBytes = this->read(buffer + totalRead, count - totalRead);
        totalRead += readBytes;
        if (totalRead == count || readBytes == 0) {
            return totalRead;
        }
    }
}

std::vector<std::uint8_t> ByteSource::readToEnd() {
    constexpr size_t chunkSize = 1024 * 8;
    auto buffer = std::vector<std::uint8_t>();

    for (auto chunkNumber = 0;; chunkNumber++) {
        buffer.resize((static_cast<decltype(buffer)::size_type>(chunkNumber) + 1) * chunkSize);
        size_t readBytes = this->readToCount(buffer.data() + (chunkNumber * chunkSize), chunkSize);

        if (readBytes < chunkSize) {
            buffer.resize(static_cast<size_t>((chunkNumber * chunkSize) + readBytes));
            return buffer;
        }
    }
}

FileContent::FileContent(std::filesystem::path path, std::size_t off, std::optional<std::size_t> length)
        : path_(std::move(path)), off_(0), length_(std::nullopt) {
    std::error_code ec;
    auto size = static_cast<std::size_t>(std::filesystem::file_size(path_, ec));
    if (!ec) {
        off_ = off;
        if (off_ > size) {
            off_ = size;
        }
        if (length.has_value()) {
            length_ = std::min(size - off_, length.value());
        } else {
            length_ = size - off_;
        }
    }
}

StreamContent::StreamContent(std::shared_ptr<std::istream> content)
        : content_(std::move(content)), length_(std::nullopt), pos_(std::streampos(-1)), seekable_(false),
          spanned_(false) {
    if (content_ != nullptr) {
        if (*(content_.get())) {
            pos_ = content_->tellg();
            if (pos_ != static_cast<std::streampos>(-1)) {
                content_->seekg(0, std::ios::end);
                if (content_->fail()) {
                    content_->clear();
                } else {
                    // seekable
                    auto lastPos = content_->tellg();
                    length_ = static_cast<std::size_t>(lastPos - pos_);
                    seekable_ = true;
                }
                content_->seekg(pos_, std::ios::beg);
            }
        }
    } else {
        // regard as empty stream
        length_ = 0;
        seekable_ = true;
    }
}

StreamContent::StreamContent(std::shared_ptr<std::istream> content, bool seekable, std::optional<std::size_t> length)
        : content_(std::move(content)), length_(length), pos_(std::streampos(-1)), seekable_(seekable),
          spanned_(false) {}


std::unique_ptr<ByteSource> StringContent::spanSource() {
    return std::make_unique<detail::MemoryByteSource>(reinterpret_cast<const std::uint8_t*>(content_.data()),
                                                      content_.size());
}

std::unique_ptr<ByteSource> FileContent::spanSource() {
    auto content = std::make_unique<std::fstream>(path_, std::ios::in | std::ios::binary);
    if (content && *content.get()) {
        if (off_ > 0) {
            content->seekg(off_, std::ios::beg);
        }
    }
    return std::make_unique<detail::StreamSource>(std::move(content));
}

std::unique_ptr<ByteSource> StreamContent::spanSource() {
    if (spanned_ && seekable_) {
        content_->clear();
        content_->seekg(pos_, std::ios::beg);
    }
    spanned_ = true;
    return std::make_unique<detail::StreamSource>(content_.get());
}

std::unique_ptr<ByteSource> MemoryContent::spanSource() {
    return std::make_unique<detail::MemoryByteSource>(reinterpret_cast<const std::uint8_t*>(content_.data()),
                                                      content_.size());
}

std::unique_ptr<ByteSource> EmptyContent::spanSource() {
    return std::make_unique<detail::MemoryByteSource>(nullptr, 0);
}

std::unique_ptr<OStreamSupplier> OStreamSupplier::from(std::function<std::shared_ptr<std::ostream>()> supplier,
                                                       bool reuse) {
    return std::make_unique<detail::OStreamSupplierImpl>(supplier, reuse);
}

} // namespace oss2
} // namespace alibabacloud