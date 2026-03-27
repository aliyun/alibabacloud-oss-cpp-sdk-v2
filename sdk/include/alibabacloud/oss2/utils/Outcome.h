
#pragma once


namespace alibabacloud {
namespace oss2 {

template <typename R, typename E>
class Outcome {
  public:
    Outcome() : result(), error(), success(false) {}
    Outcome(const R& r) : result(r), error(), success(true) {}
    Outcome(const E& e) : result(), error(e), success(false) {}
    Outcome(R&& r) : result(std::forward<R>(r)), error(), success(true) {}
    Outcome(E&& e) : result(), error(std::forward<E>(e)), success(false) {}
    Outcome(const Outcome& o) : result(o.result), error(o.error), success(o.success) {}

    template <typename RT, typename ET>
    friend class Outcome;

    template <bool B, class T = void>
    using enable_if_t = std::enable_if_t<B, T>;

    // Move both result and error from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<std::is_convertible<RT, R>::value && std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : result(std::move(o.result)), error(std::move(o.error)), success(o.success) {}

    // Move result from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<std::is_convertible<RT, R>::value && !std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : result(std::move(o.result)), success(o.success) {
        assert(o.success);
    }

    // Move error from other type of outcome
    template <typename RT, typename ET,
              enable_if_t<!std::is_convertible<RT, R>::value && std::is_convertible<ET, E>::value, int> = 0>
    Outcome(Outcome<RT, ET>&& o) : error(std::move(o.error)), success(o.success) {
        assert(!o.success);
    }

    template <typename ET, enable_if_t<std::is_convertible<ET, E>::value, int> = 0>
    Outcome(ET&& e) : error(std::forward<ET>(e)), success(false) {}

    Outcome& operator=(const Outcome& o) {
        if (this != &o) {
            result = o.result;
            error = o.error;
            success = o.success;
        }

        return *this;
    }

    Outcome(Outcome&& o)
            : // Required to force Move Constructor
              result(std::move(o.result)), error(std::move(o.error)), success(o.success) {}

    Outcome& operator=(Outcome&& o) {
        if (this != &o) {
            result = std::move(o.result);
            error = std::move(o.error);
            success = o.success;
        }

        return *this;
    }

    inline bool isSuccess() const {
        return this->success;
    }

    inline const E& getError() const {
        return error;
    }

    inline E& getError() {
        return error;
    }

    inline const R& getResult() const {
        return result;
    }

    inline R& getResult() {
        return result;
    }

  private:
    R result;
    E error;
    bool success = false;
};

} // namespace oss2
} // namespace alibabacloud