#ifndef ATOM_TYPE_EXPECTED_HPP
#define ATOM_TYPE_EXPECTED_HPP

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace atom::type {
// Generic Error class template
template <typename E>
class Error {
public:
    explicit Error(E error) : error_(std::move(error)) {}
    [[nodiscard]] auto error() const -> const E& { return error_; }

    auto operator==(const Error& other) const -> bool {
        return error_ == other.error_;
    }

private:
    E error_;
};

// Expected class template
template <typename T, typename E = std::string>
class expected {
public:
    // Constructors for success case
    expected(const T& value) : value_(value) {}
    expected(T&& value) : value_(std::move(value)) {}

    // Constructors for error case
    expected(const Error<E>& error) : value_(error) {}
    expected(Error<E>&& error) : value_(std::move(error)) {}

    // Check if the expected object contains a value
    [[nodiscard]] auto has_value() const -> bool {
        return std::holds_alternative<T>(value_);
    }

    // Retrieve the value, throws if it's an error
    auto value() -> T& {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    // Retrieve the value, throws if it's an error
    [[nodiscard]] auto value() const -> const T& {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    // Retrieve the error, throws if it's a value
    auto error() -> Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    // Retrieve the error, throws if it's a value
    [[nodiscard]] auto error() const -> const Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    template <typename U>
    auto value_or(U&& default_value) const -> T {
        return has_value() ? value()
                           : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename Func>
    auto map(Func&& func) const
        -> expected<decltype(func(std::declval<T>())), E> {
        using ReturnType = decltype(func(std::declval<T>()));
        if (has_value()) {
            return expected<ReturnType, E>(func(value()));
        } else {
            return expected<ReturnType, E>(error());
        }
    }

    template <typename Func>
    auto and_then(Func&& func) const -> decltype(func(std::declval<T>())) {
        if (has_value()) {
            return func(value());
        }
        using ReturnType = decltype(func(value()));
        return ReturnType(error());
    }

    friend auto operator==(const expected& lhs, const expected& rhs) -> bool {
        return lhs.value_ == rhs.value_;
    }

    friend auto operator!=(const expected& lhs, const expected& rhs) -> bool {
        return !(lhs == rhs);
    }

private:
    std::variant<T, Error<E>> value_;
};

// Utility functions to create expected objects
template <typename T>
auto make_expected(const T& value) -> expected<T> {
    return expected<T>(value);
}

template <typename T, typename E>
auto make_unexpected(const E& error) -> expected<T, E> {
    return expected<T, E>(Error<E>(error));
}

}  // namespace atom::type

#endif