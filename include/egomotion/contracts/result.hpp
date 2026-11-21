#pragma once

#include "egomotion/contracts/error.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace egomotion {

// Public accessors do not expose the variant, but its private storage is
// nevertheless part of this C++ ABI; no cross-toolchain stability is claimed.
template<class T, class Code>
class Result {
public:
    Result() = delete;
    Result(const Result&) = default;
    Result(Result&&) = default;
    Result& operator=(const Result&) = delete;
    Result& operator=(Result&&) = delete;

    static Result success(T value) {
        return Result(std::in_place_index<0>, std::move(value));
    }
    static Result failure(Failure<Code> why) {
        return Result(std::in_place_index<1>, std::move(why));
    }

    bool hasValue() const noexcept { return std::holds_alternative<T>(payload_); }

    const T& value() const & {
        if (!hasValue()) throw std::logic_error("Result::value(): no success value");
        return std::get<0>(payload_);
    }
    T value() && {
        if (!hasValue()) throw std::logic_error("Result::value(): no success value");
        return std::move(std::get<0>(payload_));
    }
    const T& value() const && = delete;

    const Failure<Code>& error() const & {
        if (hasValue()) throw std::logic_error("Result::error(): no failure value");
        return std::get<1>(payload_);
    }
    Failure<Code> error() && {
        if (hasValue()) throw std::logic_error("Result::error(): no failure value");
        return std::move(std::get<1>(payload_));
    }
    const Failure<Code>& error() const && = delete;

private:
    explicit Result(std::in_place_index_t<0>, T value)
        : payload_(std::in_place_index<0>, std::move(value)) {}
    explicit Result(std::in_place_index_t<1>, Failure<Code> why)
        : payload_(std::in_place_index<1>, std::move(why)) {}

    std::variant<T, Failure<Code>> payload_;
};

template<class Code>
class Status {
public:
    Status() = delete;
    Status(const Status&) = default;
    Status(Status&&) = default;
    Status& operator=(const Status&) = delete;
    Status& operator=(Status&&) = delete;

    static Status success() { return Status(std::in_place_index<0>); }
    static Status failure(Failure<Code> why) {
        return Status(std::in_place_index<1>, std::move(why));
    }

    bool hasValue() const noexcept {
        return std::holds_alternative<std::monostate>(payload_);
    }
    const Failure<Code>& error() const & {
        if (hasValue()) throw std::logic_error("Status::error(): no failure value");
        return std::get<1>(payload_);
    }
    Failure<Code> error() && {
        if (hasValue()) throw std::logic_error("Status::error(): no failure value");
        return std::move(std::get<1>(payload_));
    }
    const Failure<Code>& error() const && = delete;

private:
    explicit Status(std::in_place_index_t<0>)
        : payload_(std::in_place_index<0>) {}
    explicit Status(std::in_place_index_t<1>, Failure<Code> why)
        : payload_(std::in_place_index<1>, std::move(why)) {}

    std::variant<std::monostate, Failure<Code>> payload_;
};

}  // namespace egomotion
