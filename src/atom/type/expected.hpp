#ifndef ATOM_TYPE_EXPECTED_HPP
#define ATOM_TYPE_EXPECTED_HPP

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "atom/function/func_traits.hpp"

#include "macro.hpp"

namespace atom::type {

// Generic Error class template
template <typename E>
class Error {
public:
    explicit Error(E error) : error_(std::move(error)) {}

    // Special constructor for const char*
    template <typename T>
        requires std::is_same_v<E, std::string>
    explicit Error(const T* error) : error_(error) {}

    [[nodiscard]] auto error() const -> const E& { return error_; }

    auto operator==(const Error& other) const -> bool {
        return error_ == other.error_;
    }

private:
    E error_;
};

// unexpected class similar to std::unexpected
template <typename E>
class unexpected {
public:
    explicit unexpected(const E& error) : error_(error) {}
    explicit unexpected(E&& error) : error_(std::move(error)) {}

    [[nodiscard]] auto error() const -> const E& { return error_; }

private:
    E error_;
};

// Primary expected class template
template <typename T, typename E = std::string>
class expected {
public:
    // Constructors for success case
    expected(const T& value) : value_(value) {}
    expected(T&& value) : value_(std::move(value)) {}

    // Constructors for error case
    expected(const Error<E>& error) : value_(error) {}
    expected(Error<E>&& error) : value_(std::move(error)) {}
    expected(const unexpected<E>& unex) : value_(Error<E>(unex.error())) {}

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
        if (has_value()) {
            return value();
        } else if constexpr (std::is_invocable_v<U, E>) {
            return std::forward<U>(default_value)(error().error());
        } else {
            return static_cast<T>(std::forward<U>(default_value));
        }
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

// Specialization for void type
template <typename E>
class expected<void, E> {
public:
    // Constructors for success case
    expected() : value_(std::monostate{}) {}

    // Constructors for error case
    expected(const Error<E>& error) : value_(error) {}
    expected(Error<E>&& error) : value_(std::move(error)) {}
    expected(const unexpected<E>& unex) : value_(Error<E>(unex.error())) {}

    // Check if the expected object contains a value
    [[nodiscard]] auto has_value() const -> bool {
        return std::holds_alternative<std::monostate>(value_);
    }

    // A no-op value_or function, returns nothing as the value type is void.
    template <typename U>
    auto value_or(U&& default_value) const -> void {
        if (!has_value()) {
            std::forward<U>(default_value)(error().error());
        }
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

    template <typename Func>
    auto and_then(Func&& func) const -> expected<void, E> {
        if (has_value()) {
            func();
            return expected<void, E>();
        }
        return expected<void, E>(error());
    }

    friend auto operator==(const expected& lhs, const expected& rhs) -> bool {
        return lhs.value_ == rhs.value_;
    }

    friend auto operator!=(const expected& lhs, const expected& rhs) -> bool {
        return !(lhs == rhs);
    }

private:
    std::variant<std::monostate, Error<E>> value_;
};

// Utility functions to create expected and unexpected objects
template <typename T>
auto make_expected(T&& value) -> expected<std::decay_t<T>> {
    return expected<std::decay_t<T>>(std::forward<T>(value));
}

template <typename E>
auto make_unexpected(const E& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(error);
}

ATOM_INLINE auto make_unexpected(const char* error) -> unexpected<std::string> {
    return unexpected<std::string>(std::string(error));
}

template <typename E>
auto make_unexpected(E&& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(std::forward<E>(error));
}

}  // namespace atom::type

#endif  // ATOM_TYPE_EXPECTED_HPP
