#pragma once

#include "alibabacloud/oss2/io/ByteWriter.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace internal {

uint32_t calcCRC32(uint32_t crc, const void* buf, std::size_t len);

constexpr int FRAME_HEADER_LEN = 20;

constexpr int FRAME_TYPE_DATA = 0x800001;
constexpr int FRAME_TYPE_CONTINUOUS = 0x800004;
constexpr int FRAME_TYPE_END = 0x800005;
constexpr int FRAME_TYPE_CSV_META_END = 0x800006;
constexpr int FRAME_TYPE_JSON_META_END = 0x800007;


class SelectFrameDecodingWriter : public ByteWriter {
  public:
    explicit SelectFrameDecodingWriter(std::shared_ptr<ByteWriter> inner);

    int endStatus() const {
        return endStatus_;
    }
    const std::string& errorMessage() const {
        return errorMessage_;
    }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    int iostate() const override;

    void processBuffer();
    bool parseFrame(const std::uint8_t*& ptr, std::size_t& remain);

    std::shared_ptr<ByteWriter> inner_;
    std::vector<std::uint8_t> buffer_;

    std::uint8_t header_[FRAME_HEADER_LEN]{};
    std::int32_t headerLen_{};
    std::int32_t payloadRemains_{};
    std::uint8_t tail_[4]{};
    std::int32_t tailLen_{};
    std::uint32_t payloadCrc32_{};
    int frameType_{};

    std::vector<std::uint8_t> endFrameData_;

    int endStatus_{};
    std::string errorMessage_;
    bool error_{false};
};


class SelectMetaFrameParser : public ByteWriter {
  public:
    SelectMetaFrameParser() = default;

    std::int64_t offset() const {
        return offset_;
    }
    std::int64_t totalScanned() const {
        return totalScanned_;
    }
    std::int32_t status() const {
        return status_;
    }
    std::int32_t splitsCount() const {
        return splitsCount_;
    }
    std::int64_t rowsCount() const {
        return rowsCount_;
    }
    std::int32_t columnsCount() const {
        return columnsCount_;
    }
    const std::string& errorMessage() const {
        return errorMessage_;
    }

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override;
    int iostate() const override;

    void processBuffer();
    bool parseFrame(const std::uint8_t*& ptr, std::size_t& remain);

    std::vector<std::uint8_t> buffer_;

    std::uint8_t header_[FRAME_HEADER_LEN]{};
    std::int32_t headerLen_{};
    std::int32_t payloadRemains_{};
    std::uint8_t tail_[4]{};
    std::int32_t tailLen_{};
    std::uint32_t payloadCrc32_{};
    int frameType_{};

    std::vector<std::uint8_t> endFrameData_;

    std::int64_t offset_{};
    std::int64_t totalScanned_{};
    std::int32_t status_{};
    std::int32_t splitsCount_{};
    std::int64_t rowsCount_{};
    std::int32_t columnsCount_{};
    std::string errorMessage_;
    bool error_{false};
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
