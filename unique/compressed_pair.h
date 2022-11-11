#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S, bool F_inherritable = std::is_empty_v<F> && !std::is_final_v<F>,
          bool S_inherritable = std::is_empty_v<S> && !std::is_final_v<S>,
          bool Both = !std::is_base_of_v<S, F> && !std::is_base_of_v<F, S>>
class CompressedPair;

template <typename F, typename S, bool Both>
class CompressedPair<F, S, false, false, Both> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S, bool Both>
class CompressedPair<F, S, true, false, Both> : public F {
public:
    CompressedPair() : F(), second_() {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(const F& first, const S& second) : F(first), second_(second) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S, bool Both>
class CompressedPair<F, S, false, true, Both> : public S {
public:
    CompressedPair() : S(), first_() {
    }
    CompressedPair(F&& first, S&& second) : S(std::move(second)), first_(std::move(first)) {
    }
    CompressedPair(F&& first, const S& second) : S(second), first_(std::move(first)) {
    }
    CompressedPair(const F& first, S&& second) : S(std::move(second)), first_(first) {
    }
    CompressedPair(const F& first, const S& second) : S(second), first_(first) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, true> : public F, public S {
public:
    CompressedPair() : F(), S() {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), S(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), S(std::move(second)) {
    }
    CompressedPair(const F& first, const S& second) : F(first), S(second) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    };
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, false> : public F {
public:
    CompressedPair() : F(), second_() {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(const F& first, const S& second) : F(first), second_(second) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};