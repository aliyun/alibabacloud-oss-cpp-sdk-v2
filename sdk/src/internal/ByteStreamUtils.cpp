#include "ByteStreamUtils.h"
#include "src/utils/Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

namespace detail {

class TeeByteSource final : public ByteSource {
  public:
    TeeByteSource(std::unique_ptr<ByteSource> source, std::vector<StreamObserver*> sinks)
            : source_(std::move(source)), sinks_(std::move(sinks)) {}

  private:
    std::size_t onRead(std::uint8_t* buffer, std::size_t count) override {
        auto n = source_->read(buffer, count);
        for (auto& sink : sinks_) {
            if (sink != nullptr) {
                sink->data(buffer, n);
            }
        }
        return n;
    }

    int iostate() override {
        return source_->state();
    }

    std::unique_ptr<ByteSource> source_;
    std::vector<StreamObserver*> sinks_;
};
} // namespace detail

void TeeByteContent::resetObserver() {
    for (auto& sink : sinks_) {
        if (sink == nullptr) {
            continue;
        }
        sink->reset();
    }
}

std::vector<StreamObserver*> TeeByteContent::spanObserver() {
    std::vector<StreamObserver*> observers{};
    for (auto& sink : sinks_) {
        if (sink == nullptr) {
            continue;
        }
        observers.push_back(sink.get());
    }
    return observers;
}

std::unique_ptr<ByteSource> TeeByteContent::spanSource() {
    if (!first_) {
        resetObserver();
    }
    first_ = false;
    return std::make_unique<detail::TeeByteSource>(source_->spanSource(), spanObserver());
}


void CRC64Observer::data(std::uint8_t* buffer, std::size_t count) {
    value_ = utils::CalcCRC64(value_, buffer, count);
}


} // namespace internal
} // namespace oss2
} // namespace alibabacloud