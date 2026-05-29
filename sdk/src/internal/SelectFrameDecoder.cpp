
#include "SelectFrameDecoder.h"

#include <algorithm>
#include <cstring>
#include <ios>

namespace alibabacloud {
namespace oss2 {
namespace internal {

static const uint32_t crc32Table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
    0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
    0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
    0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
    0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
    0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
    0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525,
    0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
    0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
    0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6,
    0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
    0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7,
    0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
    0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330,
    0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

uint32_t calcCRC32(uint32_t crc, const void* buf, std::size_t len) {
    auto bytes = static_cast<const uint8_t*>(buf);
    crc ^= 0xFFFFFFFF;
    for (std::size_t i = 0; i < len; i++) {
        crc = (crc >> 8) ^ crc32Table[(crc ^ bytes[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

static inline uint32_t readBE32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16)
        | (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

static inline int32_t readBE32Signed(const uint8_t* p) {
    return static_cast<int32_t>(readBE32(p));
}

static inline int64_t readBE64Signed(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v = (v << 8) | p[i];
    }
    return static_cast<int64_t>(v);
}


// --- SelectFrameDecodingWriter ---

SelectFrameDecodingWriter::SelectFrameDecodingWriter(std::shared_ptr<ByteWriter> inner) : inner_(std::move(inner)) {}

std::size_t SelectFrameDecodingWriter::onWrite(const std::uint8_t* data, std::size_t n) {
    if (error_) {
        return 0;
    }
    buffer_.insert(buffer_.end(), data, data + n);
    processBuffer();
    if (error_) {
        return 0;
    }
    return n;
}

int SelectFrameDecodingWriter::iostate() const {
    if (error_) {
        return std::ios_base::failbit;
    }
    if (inner_) {
        return inner_->state();
    }
    return 0;
}

void SelectFrameDecodingWriter::processBuffer() {
    const std::uint8_t* ptr = buffer_.data();
    std::size_t remain = buffer_.size();

    while (remain > 0 && !error_) {
        if (!parseFrame(ptr, remain)) {
            break;
        }
    }

    if (remain > 0 && remain < buffer_.size()) {
        std::vector<std::uint8_t> leftover(ptr, ptr + remain);
        buffer_.swap(leftover);
    } else if (remain == 0) {
        buffer_.clear();
    }
}

bool SelectFrameDecodingWriter::parseFrame(const std::uint8_t*& ptr, std::size_t& remain) {
    // Phase 1: accumulate header
    if (headerLen_ < FRAME_HEADER_LEN) {
        auto need = static_cast<std::size_t>(FRAME_HEADER_LEN - headerLen_);
        auto copy = std::min(need, remain);
        std::memcpy(header_ + headerLen_, ptr, copy);
        headerLen_ += static_cast<std::int32_t>(copy);
        ptr += copy;
        remain -= copy;

        if (headerLen_ == FRAME_HEADER_LEN) {
            uint32_t payloadLength = readBE32(header_ + 4);
            payloadRemains_ = static_cast<std::int32_t>(payloadLength) - 8;
            frameType_ =
                static_cast<int>((static_cast<uint32_t>(header_[1]) << 16) | (static_cast<uint32_t>(header_[2]) << 8)
                                 | static_cast<uint32_t>(header_[3]));
            payloadCrc32_ = calcCRC32(0, header_ + 12, 8);
            endFrameData_.clear();
        }
        if (headerLen_ < FRAME_HEADER_LEN) {
            return false;
        }
    }

    // Phase 2: payload
    if (payloadRemains_ > 0) {
        auto copy = std::min(static_cast<std::size_t>(payloadRemains_), remain);
        if (copy == 0) {
            return false;
        }

        payloadCrc32_ = calcCRC32(payloadCrc32_, ptr, copy);

        if (frameType_ == FRAME_TYPE_DATA) {
            auto written = inner_->write(ptr, copy);
            if (written == 0 && inner_->fail()) {
                error_ = true;
                return false;
            }
        } else if (frameType_ == FRAME_TYPE_END) {
            endFrameData_.insert(endFrameData_.end(), ptr, ptr + copy);
        }

        ptr += copy;
        remain -= copy;
        payloadRemains_ -= static_cast<std::int32_t>(copy);
        if (payloadRemains_ > 0) {
            return false;
        }
    }

    // Phase 3: tail (4 bytes checksum)
    if (tailLen_ < 4) {
        auto need = static_cast<std::size_t>(4 - tailLen_);
        auto copy = std::min(need, remain);
        if (copy == 0) {
            return false;
        }
        std::memcpy(tail_ + tailLen_, ptr, copy);
        tailLen_ += static_cast<std::int32_t>(copy);
        ptr += copy;
        remain -= copy;
        if (tailLen_ < 4) {
            return false;
        }
    }

    // Validate CRC
    uint32_t expectedCrc = readBE32(tail_);
    if (expectedCrc != 0 && expectedCrc != payloadCrc32_) {
        error_ = true;
        errorMessage_ = "CRC32 checksum mismatch";
        return false;
    }

    // End frame payload: totalScanned(8B) + status(4B) + ...
    if (frameType_ == FRAME_TYPE_END && endFrameData_.size() >= 12) {
        endStatus_ = readBE32Signed(endFrameData_.data() + 8);
    }

    // Reset for next frame
    headerLen_ = 0;
    tailLen_ = 0;
    payloadRemains_ = 0;
    frameType_ = 0;

    return true;
}


// --- SelectMetaFrameParser ---

std::size_t SelectMetaFrameParser::onWrite(const std::uint8_t* data, std::size_t n) {
    if (error_) {
        return 0;
    }
    buffer_.insert(buffer_.end(), data, data + n);
    processBuffer();
    if (error_) {
        return 0;
    }
    return n;
}

int SelectMetaFrameParser::iostate() const {
    return error_ ? std::ios_base::failbit : 0;
}

void SelectMetaFrameParser::processBuffer() {
    const std::uint8_t* ptr = buffer_.data();
    std::size_t remain = buffer_.size();

    while (remain > 0 && !error_) {
        if (!parseFrame(ptr, remain)) {
            break;
        }
    }

    if (remain > 0 && remain < buffer_.size()) {
        std::vector<std::uint8_t> leftover(ptr, ptr + remain);
        buffer_.swap(leftover);
    } else if (remain == 0) {
        buffer_.clear();
    }
}

bool SelectMetaFrameParser::parseFrame(const std::uint8_t*& ptr, std::size_t& remain) {
    // Phase 1: accumulate header
    if (headerLen_ < FRAME_HEADER_LEN) {
        auto need = static_cast<std::size_t>(FRAME_HEADER_LEN - headerLen_);
        auto copy = std::min(need, remain);
        std::memcpy(header_ + headerLen_, ptr, copy);
        headerLen_ += static_cast<std::int32_t>(copy);
        ptr += copy;
        remain -= copy;

        if (headerLen_ == FRAME_HEADER_LEN) {
            uint32_t payloadLength = readBE32(header_ + 4);
            payloadRemains_ = static_cast<std::int32_t>(payloadLength) - 8;
            frameType_ =
                static_cast<int>((static_cast<uint32_t>(header_[1]) << 16) | (static_cast<uint32_t>(header_[2]) << 8)
                                 | static_cast<uint32_t>(header_[3]));
            payloadCrc32_ = calcCRC32(0, header_ + 12, 8);
            endFrameData_.clear();
        }
        if (headerLen_ < FRAME_HEADER_LEN) {
            return false;
        }
    }

    // Phase 2: payload
    if (payloadRemains_ > 0) {
        auto copy = std::min(static_cast<std::size_t>(payloadRemains_), remain);
        if (copy == 0) {
            return false;
        }

        payloadCrc32_ = calcCRC32(payloadCrc32_, ptr, copy);

        if (frameType_ == FRAME_TYPE_CSV_META_END || frameType_ == FRAME_TYPE_JSON_META_END
            || frameType_ == FRAME_TYPE_END) {
            endFrameData_.insert(endFrameData_.end(), ptr, ptr + copy);
        }

        ptr += copy;
        remain -= copy;
        payloadRemains_ -= static_cast<std::int32_t>(copy);
        if (payloadRemains_ > 0) {
            return false;
        }
    }

    // Phase 3: tail (4 bytes checksum)
    if (tailLen_ < 4) {
        auto need = static_cast<std::size_t>(4 - tailLen_);
        auto copy = std::min(need, remain);
        if (copy == 0) {
            return false;
        }
        std::memcpy(tail_ + tailLen_, ptr, copy);
        tailLen_ += static_cast<std::int32_t>(copy);
        ptr += copy;
        remain -= copy;
        if (tailLen_ < 4) {
            return false;
        }
    }

    // Validate CRC
    uint32_t expectedCrc = readBE32(tail_);
    if (expectedCrc != 0 && expectedCrc != payloadCrc32_) {
        error_ = true;
        errorMessage_ = "CRC32 checksum mismatch";
        return false;
    }

    // Extract metadata from meta end frames
    // Frame payload layout (after the 8-byte offset in header):
    //   CSV meta end (0x800006): totalScanned(8B) + status(4B) + splitsCount(4B) + rowsCount(8B) + columnsCount(4B) +
    //   errorMessage JSON meta end (0x800007): totalScanned(8B) + status(4B) + splitsCount(4B) + rowsCount(8B) +
    //   errorMessage End frame (0x800005): same as JSON meta end

    offset_ = readBE64Signed(header_ + 12);

    if (frameType_ == FRAME_TYPE_CSV_META_END && endFrameData_.size() >= 28) {
        totalScanned_ = readBE64Signed(endFrameData_.data());
        status_ = readBE32Signed(endFrameData_.data() + 8);
        splitsCount_ = readBE32Signed(endFrameData_.data() + 12);
        rowsCount_ = readBE64Signed(endFrameData_.data() + 16);
        columnsCount_ = readBE32Signed(endFrameData_.data() + 24);
        if (endFrameData_.size() > 28) {
            errorMessage_.assign(reinterpret_cast<const char*>(endFrameData_.data() + 28), endFrameData_.size() - 28);
        }
    } else if ((frameType_ == FRAME_TYPE_JSON_META_END || frameType_ == FRAME_TYPE_END) && endFrameData_.size() >= 24) {
        totalScanned_ = readBE64Signed(endFrameData_.data());
        status_ = readBE32Signed(endFrameData_.data() + 8);
        splitsCount_ = readBE32Signed(endFrameData_.data() + 12);
        rowsCount_ = readBE64Signed(endFrameData_.data() + 16);
        if (endFrameData_.size() > 24) {
            errorMessage_.assign(reinterpret_cast<const char*>(endFrameData_.data() + 24), endFrameData_.size() - 24);
        }
    }

    // Reset for next frame
    headerLen_ = 0;
    tailLen_ = 0;
    payloadRemains_ = 0;
    frameType_ = 0;
    endFrameData_.clear();

    return true;
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
