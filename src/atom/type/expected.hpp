#ifndef ATOM_TYPE_EXPECTED_HPP
#define ATOM_TYPE_EXPECTED_HPP

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace atom::type {

/**
 * @brief Generic Error class template.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class Error {
public:
    /**
     * @brief Constructs an Error with the given error value.
     *
     * @param error The error value.
     */
    explicit Error(E error) : error_(std::move(error)) {}

    /**
     * @brief Special constructor for const char* when E is std::string.
     *
     * @param error The error message.
     */
    template <typename T>
        requires std::is_same_v<E, std::string>
    explicit Error(const T* error) : error_(error) {}

    /**
     * @brief Retrieves the error value.
     *
     * @return The error value.
     */
    [[nodiscard]] auto error() const -> const E& { return error_; }

    /**
     * @brief Equality operator for Error.
     *
     * @param other The other Error to compare with.
     * @return True if the errors are equal, false otherwise.
     */
    auto operator==(const Error& other) const -> bool {
        return error_ == other.error_;
    }

private:
    E error_;  ///< The error value.
};

/**
 * @brief unexpected class similar to std::unexpected.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class unexpected {
public:
    /**
     * @brief Constructs an unexpected with the given error value.
     *
     * @param error The error value.
     */
    explicit unexpected(const E& error) : error_(error) {}

    /**
     * @brief Constructs an unexpected with the given error value.
     *
     * @param error The error value.
     */
    explicit unexpected(E&& error) : error_(std::move(error)) {}

    /**
     * @brief Retrieves the error value.
     *
     * @return The error value.
     */
    [[nodiscard]] auto error() const -> const E& { return error_; }

private:
    E error_;  ///< The error value.
};

/**
 * @brief Primary expected class template.
 *
 * @tparam T The type of the value.
 * @tparam E The type of the error (default is std::string).
 */
template <typename T, typename E = std::string>
class expected {
public:
    /**
     * @brief Constructs an expected with the given value.
     *
     * @param value The value.
     */
    expected(const T& value) : value_(value) {}

    /**
     * @brief Constructs an expected with the given value.
     *
     * @param value The value.
     */
    expected(T&& value) : value_(std::move(value)) {}

    /**
     * @brief Constructs an expected with the given error.
     *
     * @param error The error.
     */
    expected(const Error<E>& error) : value_(error) {}

    /**
     * @brief Constructs an expected with the given error.
     *
     * @param error The error.
     */
    expected(Error<E>&& error) : value_(std::move(error)) {}

    /**
     * @brief Constructs an expected with the given unexpected error.
     *
     * @param unex The unexpected error.
     */
    expected(const unexpected<E>& unex) : value_(Error<E>(unex.error())) {}

    /**
     * @brief Checks if the expected object contains a value.
     *
     * @return True if it contains a value, false otherwise.
     */
    [[nodiscard]] auto has_value() const -> bool {
        return std::holds_alternative<T>(value_);
    }

    /**
     * @brief Retrieves the value, throws if it's an error.
     *
     * @return The value.
     * @throws std::logic_error if it contains an error.
     */
    auto value() -> T& {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    /**
     * @brief Retrieves the value, throws if it's an error.
     *
     * @return The value.
     * @throws std::logic_error if it contains an error.
     */
    [[nodiscard]] auto value() const -> const T& {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    /**
     * @brief Retrieves the error, throws if it's a value.
     *
     * @return The error.
     * @throws std::logic_error if it contains a value.
     */
    auto error() -> Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves the error, throws if it's a value.
     *
     * @return The error.
     * @throws std::logic_error if it contains a value.
     */
    [[nodiscard]] auto error() const -> const Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves the value or a default value if it contains an error.
     *
     * @tparam U The type of the default value.
     * @param default_value The default value.
     * @return The value or the default value.
     */
    template <typename U>
    auto value_or(U&& default_value) const -> T {
        if (has_value()) {
            return value();
        }
        if constexpr (std::is_invocable_v<U, E>) {
            return std::forward<U>(default_value)(error().error());
        } else {
            return static_cast<T>(std::forward<U>(default_value));
        }
    }

    /**
     * @brief Maps the value to another type using the given function.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the value.
     * @return An expected object with the mapped value or the original error.
     */
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

    /**
     * @brief Applies the given function to the value if it exists.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the value.
     * @return The result of the function or the original error.
     */
    template <typename Func>
    auto and_then(Func&& func) const -> decltype(func(std::declval<T>())) {
        if (has_value()) {
            return func(value());
        }
        using ReturnType = decltype(func(value()));
        return ReturnType(error());
    }

    /**
     * @brief Transforms the error using the given function.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the error.
     * @return An expected object with the original value or the transformed
     * error.
     */
    template <typename Func>
    auto transform_error(Func&& func) const
        -> expected<T, decltype(func(std::declval<E>()))> {
        using ErrorType = decltype(func(std::declval<E>()));
        if (has_value()) {
            return expected<T, ErrorType>(value());
        }
        return expected<T, ErrorType>(Error<ErrorType>(func(error().error())));
    }

    /**
     * @brief Applies the given function to the error if it exists.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the error.
     * @return An expected object with the original value or the result of the
     * function.
     */
    template <typename Func>
    auto or_else(Func&& func) const -> expected<T, E> {
        if (has_value()) {
            return *this;
        }
        return func(error().error());
    }

    /**
     * @brief Equality operator for expected.
     *
     * @param lhs The left-hand side expected.
     * @param rhs The right-hand side expected.
     * @return True if the expected objects are equal, false otherwise.
     */
    friend auto operator==(const expected& lhs, const expected& rhs) -> bool {
        return lhs.value_ == rhs.value_;
    }

    /**
     * @brief Inequality operator for expected.
     *
     * @param lhs The left-hand side expected.
     * @param rhs The right-hand side expected.
     * @return True if the expected objects are not equal, false otherwise.
     */
    friend auto operator!=(const expected& lhs, const expected& rhs) -> bool {
        return !(lhs == rhs);
    }

private:
    std::variant<T, Error<E>> value_;  ///< The value or error.
};

/**
 * @brief Specialization of expected for void type.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class expected<void, E> {
public:
    /**
     * @brief Constructs an expected with a void value.
     */
    expected() : value_(std::monostate{}) {}

    /**
     * @brief Constructs an expected with the given error.
     *
     * @param error The error.
     */
    expected(const Error<E>& error) : value_(error) {}

    /**
     * @brief Constructs an expected with the given error.
     *
     * @param error The error.
     */
    expected(Error<E>&& error) : value_(std::move(error)) {}

    /**
     * @brief Constructs an expected with the given unexpected error.
     *
     * @param unex The unexpected error.
     */
    expected(const unexpected<E>& unex) : value_(Error<E>(unex.error())) {}

    /**
     * @brief Checks if the expected object contains a value.
     *
     * @return True if it contains a value, false otherwise.
     */
    [[nodiscard]] auto has_value() const -> bool {
        return std::holds_alternative<std::monostate>(value_);
    }

    /**
     * @brief A no-op value_or function, returns nothing as the value type is
     * void.
     *
     * @tparam U The type of the default value.
     * @param default_value The default value.
     */
    template <typename U>
    auto value_or(U&& default_value) const -> void {
        if (!has_value()) {
            std::forward<U>(default_value)(error().error());
        }
    }

    /**
     * @brief Retrieves the error, throws if it's a value.
     *
     * @return The error.
     * @throws std::logic_error if it contains a value.
     */
    auto error() -> Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves the error, throws if it's a value.
     *
     * @return The error.
     * @throws std::logic_error if it contains a value.
     */
    [[nodiscard]] auto error() const -> const Error<E>& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Applies the given function if it contains a value.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply.
     * @return An expected object with the result of the function or the
     * original error.
     */
    template <typename Func>
    auto and_then(Func&& func) const -> expected<void, E> {
        if (has_value()) {
            func();
            return expected<void, E>();
        }
        return expected<void, E>(error());
    }

    /**
     * @brief Transforms the error using the given function.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the error.
     * @return An expected object with the original value or the transformed
     * error.
     */
    template <typename Func>
    auto transform_error(Func&& func) const
        -> expected<void, decltype(func(std::declval<E>()))> {
        using ErrorType = decltype(func(std::declval<E>()));
        if (has_value()) {
            return expected<void, ErrorType>();
        }
        return expected<void, ErrorType>(
            Error<ErrorType>(func(error().error())));
    }

    /**
     * @brief Applies the given function to the error if it exists.
     *
     * @tparam Func The type of the function.
     * @param func The function to apply to the error.
     * @return An expected object with the original value or the result of the
     * function.
     */
    template <typename Func>
    auto or_else(Func&& func) const -> expected<void, E> {
        if (has_value()) {
            return *this;
        }
        return func(error().error());
    }

    /**
     * @brief Equality operator for expected.
     *
     * @param lhs The left-hand side expected.
     * @param rhs The right-hand side expected.
     * @return True if the expected objects are equal, false otherwise.
     */
    friend auto operator==(const expected& lhs, const expected& rhs) -> bool {
        return lhs.value_ == rhs.value_;
    }

    /**
     * @brief Inequality operator for expected.
     *
     * @param lhs The left-hand side expected.
     * @param rhs The right-hand side expected.
     * @return True if the expected objects are not equal, false otherwise.
     */
    friend auto operator!=(const expected& lhs, const expected& rhs) -> bool {
        return !(lhs == rhs);
    }

private:
    std::variant<std::monostate, Error<E>> value_;  ///< The value or error.
};

/**
 * @brief Utility function to create an expected object.
 *
 * @tparam T The type of the value.
 * @param value The value.
 * @return An expected object containing the value.
 */
template <typename T>
auto make_expected(T&& value) -> expected<std::decay_t<T>> {
    return expected<std::decay_t<T>>(std::forward<T>(value));
}

/**
 * @brief Utility function to create an unexpected object.
 *
 * @tparam E The type of the error.
 * @param error The error.
 * @return An unexpected object containing the error.
 */
template <typename E>
auto make_unexpected(const E& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(error);
}

/**
 * @brief Utility function to create an unexpected object from a const char*.
 *
 * @param error The error message.
 * @return An unexpected object containing the error message.
 */
auto make_unexpected(const char* error) -> unexpected<std::string> {
    return unexpected<std::string>(std::string(error));
}

/**
 * @brief Utility function to create an unexpected object.
 *
 * @tparam E The type of the error.
 * @param error The error.
 * @return An unexpected object containing the error.
 */
template <typename E>
auto make_unexpected(E&& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(std::forward<E>(error));
}

}  // namespace atom::type

#endif  // ATOM_TYPE_EXPECTED_HPP