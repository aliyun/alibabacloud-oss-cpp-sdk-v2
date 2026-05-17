#pragma once

#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_USE_STD_EXPECTED
#include <expected>
#endif

namespace alibabacloud {
namespace oss2 {

#ifdef ALIBABACLOUD_OSS_USE_STD_EXPECTED

template <typename R, typename E>
using Outcome = std::expected<R, E>;

template <typename E>
std::unexpected<std::decay_t<E>> makeUnexpected(E&& e) {
    return std::unexpected<std::decay_t<E>>(std::forward<E>(e));
}

#else

template <typename R, typename E>
class Outcome {
  public:
    Outcome() : result_(), error_(), hasValue_(false) {}
    Outcome(const R& r) : result_(r), error_(), hasValue_(true) {}
    Outcome(const E& e) : result_(), error_(e), hasValue_(false) {}
    Outcome(R&& r) : result_(std::forward<R>(r)), error_(), hasValue_(true) {}
    Outcome(E&& e) : result_(), error_(std::forward<E>(e)), hasValue_(false) {}
    Outcome(const Outcome& o) : result_(o.result_), error_(o.error_), hasValue_(o.hasValue_) {}

    template <typename RT, typename ET>
    friend class Outcome;

    template <bool B, class T = void>
    using enable_if_t = std::enable_if_t<B, T>;

    // Move both result and error from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<std::is_convertible<RT, R>::value && std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : result_(std::move(o.result_)), error_(std::move(o.error_)), hasValue_(o.hasValue_) {}

    // Move result from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<std::is_convertible<RT, R>::value && !std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : result_(std::move(o.result_)), hasValue_(o.hasValue_) {
        assert(o.hasValue_);
    }

    // Move error from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<!std::is_convertible<RT, R>::value && std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : error_(std::move(o.error_)), hasValue_(o.hasValue_) {
        assert(!o.hasValue_);
    }

    template <typename ET, enable_if_t<std::is_convertible<ET, E>::value, int> = 0>
    Outcome(ET&& e) : error_(std::forward<ET>(e)), hasValue_(false) {}

    Outcome& operator=(const Outcome& o) {
        if (this != &o) {
            result_ = o.result_;
            error_ = o.error_;
            hasValue_ = o.hasValue_;
        }

        return *this;
    }

    Outcome(Outcome&& o)
            : // Required to force Move Constructor
              result_(std::move(o.result_)), error_(std::move(o.error_)), hasValue_(o.hasValue_) {}

    Outcome& operator=(Outcome&& o) {
        if (this != &o) {
            result_ = std::move(o.result_);
            error_ = std::move(o.error_);
            hasValue_ = o.hasValue_;
        }

        return *this;
    }

    // --- std::expected compatible interface ---

    bool has_value() const { return hasValue_; }
    explicit operator bool() const { return hasValue_; }

    R& value() { return result_; }
    const R& value() const { return result_; }

    E& error() { return error_; }
    const E& error() const { return error_; }

    R& operator*() { return result_; }
    const R& operator*() const { return result_; }
    R* operator->() { return &result_; }
    const R* operator->() const { return &result_; }

    template <typename U>
    R value_or(U&& default_value) const {
        return hasValue_ ? result_ : static_cast<R>(std::forward<U>(default_value));
    }

    // --- Legacy interface (compatible with aliyun-oss-cpp-sdk) ---

    bool isSuccess() const { return hasValue_; }

    const E& getError() const { return error_; }
    E& getError() { return error_; }

    const R& getResult() const { return result_; }
    R& getResult() { return result_; }

  private:
    R result_;
    E error_;
    bool hasValue_ = false;
};

template <typename E>
E&& makeUnexpected(E&& e) { return std::forward<E>(e); }

#endif // !ALIBABACLOUD_OSS_USE_STD_EXPECTED

} // namespace oss2
} // namespace alibabacloud
