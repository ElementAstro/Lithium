#ifndef ATOM_TYPE_EXPECTED_HPP
#define ATOM_TYPE_EXPECTED_HPP

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace atom::type {

/**
 * @brief A generic error class template that encapsulates error information.
 *
 * The `Error` class is used to represent and store error details. It provides
 * access to the error and supports comparison operations.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class Error {
public:
    /**
     * @brief Constructs an `Error` object with the given error.
     *
     * @param error The error to be stored.
     */
    explicit Error(E error) : error_(std::move(error)) {}

    /**
     * @brief Constructs an `Error` object from a C-style string if the error
     * type is `std::string`.
     *
     * @tparam T The type of the C-style string.
     * @param error The C-style string representing the error.
     */
    template <typename T>
        requires std::is_same_v<E, std::string>
    explicit Error(const T* error) : error_(error) {}

    /**
     * @brief Retrieves the stored error.
     *
     * @return A constant reference to the stored error.
     */
    [[nodiscard]] auto error() const -> const E& { return error_; }

    /**
     * @brief Compares two `Error` objects for equality.
     *
     * @param other The other `Error` object to compare with.
     * @return `true` if both errors are equal, `false` otherwise.
     */
    auto operator==(const Error& other) const -> bool {
        return error_ == other.error_;
    }

private:
    E error_;  ///< The encapsulated error.
};

/**
 * @brief An `unexpected` class template similar to `std::unexpected`.
 *
 * The `unexpected` class is used to represent an error state in the `expected`
 * type.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class unexpected {
public:
    /**
     * @brief Constructs an `unexpected` object with a constant reference to an
     * error.
     *
     * @param error The error to be stored.
     */
    explicit unexpected(const E& error) : error_(error) {}

    /**
     * @brief Constructs an `unexpected` object by moving an error.
     *
     * @param error The error to be stored.
     */
    explicit unexpected(E&& error) : error_(std::move(error)) {}

    /**
     * @brief Retrieves the stored error.
     *
     * @return A constant reference to the stored error.
     */
    [[nodiscard]] auto error() const -> const E& { return error_; }

    /**
     * @brief Compares two `unexpected` objects for equality.
     *
     * @param other The other `unexpected` object to compare with.
     * @return `true` if both errors are equal, `false` otherwise.
     */
    bool operator==(const unexpected& other) const {
        return error_ == other.error_;
    }

private:
    E error_;  ///< The encapsulated error.
};

/**
 * @brief The primary `expected` class template.
 *
 * The `expected` class represents a value that may either contain a valid value
 * of type `T` or an error of type `E`. It provides mechanisms to access the
 * value or the error and supports various monadic operations.
 *
 * @tparam T The type of the expected value.
 * @tparam E The type of the error (default is `std::string`).
 */
template <typename T, typename E = std::string>
class expected {
public:
    // Constructors for value

    /**
     * @brief Default constructs an `expected` object containing a
     * default-constructed value.
     *
     * This constructor is only enabled if `T` is default constructible.
     */
    constexpr expected()
        requires std::is_default_constructible_v<T>
        : value_(std::in_place_index<0>, T()) {}

    /**
     * @brief Constructs an `expected` object containing a copy of the given
     * value.
     *
     * @param value The value to be stored.
     */
    constexpr expected(const T& value)
        : value_(std::in_place_index<0>, value) {}

    /**
     * @brief Constructs an `expected` object containing a moved value.
     *
     * @param value The value to be moved and stored.
     */
    constexpr expected(T&& value)
        : value_(std::in_place_index<0>, std::move(value)) {}

    // Constructors for error

    /**
     * @brief Constructs an `expected` object containing a copy of the given
     * error.
     *
     * @param error The error to be stored.
     */
    constexpr expected(const Error<E>& error) : value_(error) {}

    /**
     * @brief Constructs an `expected` object containing a moved error.
     *
     * @param error The error to be moved and stored.
     */
    constexpr expected(Error<E>&& error) : value_(std::move(error)) {}

    /**
     * @brief Constructs an `expected` object from an `unexpected` error by
     * copying it.
     *
     * @param unex The `unexpected` error to be stored.
     */
    constexpr expected(const unexpected<E>& unex)
        : value_(Error<E>(unex.error())) {}

    /**
     * @brief Constructs an `expected` object from an `unexpected` error by
     * moving it.
     *
     * @param unex The `unexpected` error to be moved and stored.
     */
    constexpr expected(unexpected<E>&& unex)
        : value_(Error<E>(std::move(unex.error()))) {}

    // Copy and move constructors

    /**
     * @brief Default copy constructor.
     */
    constexpr expected(const expected&) = default;

    /**
     * @brief Default move constructor.
     */
    constexpr expected(expected&&) noexcept = default;

    /**
     * @brief Default copy assignment operator.
     */
    constexpr expected& operator=(const expected&) = default;

    /**
     * @brief Default move assignment operator.
     */
    constexpr expected& operator=(expected&&) noexcept = default;

    // Observers

    /**
     * @brief Checks if the `expected` object contains a valid value.
     *
     * @return `true` if it contains a value, `false` if it contains an error.
     */
    [[nodiscard]] constexpr bool has_value() const noexcept {
        return std::holds_alternative<T>(value_);
    }

    /**
     * @brief Retrieves a reference to the stored value.
     *
     * @return A reference to the stored value.
     * @throws std::logic_error If the `expected` contains an error.
     */
    [[nodiscard]] constexpr T& value() & {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    /**
     * @brief Retrieves a constant reference to the stored value.
     *
     * @return A constant reference to the stored value.
     * @throws std::logic_error If the `expected` contains an error.
     */
    [[nodiscard]] constexpr const T& value() const& {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(value_);
    }

    /**
     * @brief Retrieves an rvalue reference to the stored value.
     *
     * @return An rvalue reference to the stored value.
     * @throws std::logic_error If the `expected` contains an error.
     */
    [[nodiscard]] constexpr T&& value() && {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
        return std::get<T>(std::move(value_));
    }

    /**
     * @brief Retrieves a constant reference to the stored error.
     *
     * @return A constant reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr const Error<E>& error() const& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves a reference to the stored error.
     *
     * @return A reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr Error<E>& error() & {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves an rvalue reference to the stored error.
     *
     * @return An rvalue reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr Error<E>&& error() && {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(std::move(value_));
    }

    /**
     * @brief Conversion operator to `bool`.
     *
     * @return `true` if the `expected` contains a value, `false` otherwise.
     */
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // Monadic operations

    /**
     * @brief Applies a function to the stored value if it exists.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(
        Func&& func) & -> decltype(func(std::declval<T&>())) {
        if (has_value()) {
            return func(value());
        }
        return decltype(func(std::declval<T&>()))(error());
    }

    /**
     * @brief Applies a constant function to the stored value if it exists.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(
        Func&& func) const& -> decltype(func(std::declval<const T&>())) {
        if (has_value()) {
            return func(value());
        }
        return decltype(func(std::declval<const T&>()))(error());
    }

    /**
     * @brief Applies a function to the stored value if it exists, moving the
     * value.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(
        Func&& func) && -> decltype(func(std::declval<T&&>())) {
        if (has_value()) {
            return func(std::move(value()));
        }
        return decltype(func(std::declval<T&&>()))(std::move(error()));
    }

    /**
     * @brief Applies a function to the stored value if it exists and wraps the
     * result in an `expected`.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return An `expected` containing the result of the function, or an
     * `expected` containing the error.
     */
    template <typename Func>
    constexpr auto map(
        Func&& func) & -> expected<decltype(func(std::declval<T&>())), E> {
        if (has_value()) {
            return expected<decltype(func(value())), E>(func(value()));
        }
        return expected<decltype(func(std::declval<T&>())), E>(error());
    }

    /**
     * @brief Applies a constant function to the stored value if it exists and
     * wraps the result in an `expected`.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return An `expected` containing the result of the function, or an
     * `expected` containing the error.
     */
    template <typename Func>
    constexpr auto map(Func&& func)
        const& -> expected<decltype(func(std::declval<const T&>())), E> {
        if (has_value()) {
            return expected<decltype(func(value())), E>(func(value()));
        }
        return expected<decltype(func(std::declval<const T&>())), E>(error());
    }

    /**
     * @brief Applies a function to the stored value if it exists, moving the
     * value, and wraps the result in an `expected`.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to the stored value.
     * @return An `expected` containing the result of the function, or an
     * `expected` containing the error.
     */
    template <typename Func>
    constexpr auto map(
        Func&& func) && -> expected<decltype(func(std::declval<T&&>())), E> {
        if (has_value()) {
            return expected<decltype(func(std::declval<T&&>())), E>(
                func(std::move(value())));
        }
        return expected<decltype(func(std::declval<T&&>())), E>(
            std::move(error()));
    }

    /**
     * @brief Transforms the stored error using the provided function.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(
        Func&& func) & -> expected<T, decltype(func(std::declval<E&>()))> {
        if (has_value()) {
            return *this;
        }
        return expected<T, decltype(func(std::declval<E&>()))>(
            func(error().error()));
    }

    /**
     * @brief Transforms the stored error using the provided constant function.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(Func&& func)
        const& -> expected<T, decltype(func(std::declval<const E&>()))> {
        if (has_value()) {
            return *this;
        }
        return expected<T, decltype(func(std::declval<const E&>()))>(
            func(error().error()));
    }

    /**
     * @brief Transforms the stored error using the provided function, moving
     * the error.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(
        Func&& func) && -> expected<T, decltype(func(std::declval<E&&>()))> {
        if (has_value()) {
            return std::move(*this);
        }
        return expected<T, decltype(func(std::declval<E&&>()))>(
            func(std::move(error().error())));
    }

    // Equality operators

    /**
     * @brief Compares two `expected` objects for equality.
     *
     * @param lhs The left-hand side `expected` object.
     * @param rhs The right-hand side `expected` object.
     * @return `true` if both `expected` objects are equal, `false` otherwise.
     */
    friend constexpr bool operator==(const expected& lhs, const expected& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;
        if (lhs.has_value()) {
            return lhs.value_ == rhs.value_;
        }
        return lhs.error_ == rhs.error_;
    }

    /**
     * @brief Compares two `expected` objects for inequality.
     *
     * @param lhs The left-hand side `expected` object.
     * @param rhs The right-hand side `expected` object.
     * @return `true` if both `expected` objects are not equal, `false`
     * otherwise.
     */
    friend constexpr bool operator!=(const expected& lhs, const expected& rhs) {
        return !(lhs == rhs);
    }

private:
    std::variant<T, Error<E>>
        value_;  ///< The variant holding either the value or the error.
};

/**
 * @brief Specialization of the `expected` class template for `void` type.
 *
 * This specialization handles cases where no value is expected, only an error.
 *
 * @tparam E The type of the error.
 */
template <typename E>
class expected<void, E> {
public:
    // Constructors for value

    /**
     * @brief Default constructs an `expected` object containing no value.
     */
    constexpr expected() noexcept : value_(std::monostate{}) {}

    // Constructors for error

    /**
     * @brief Constructs an `expected` object containing a copy of the given
     * error.
     *
     * @param error The error to be stored.
     */
    constexpr expected(const Error<E>& error) : value_(error) {}

    /**
     * @brief Constructs an `expected` object containing a moved error.
     *
     * @param error The error to be moved and stored.
     */
    constexpr expected(Error<E>&& error) : value_(std::move(error)) {}

    /**
     * @brief Constructs an `expected` object from an `unexpected` error by
     * copying it.
     *
     * @param unex The `unexpected` error to be stored.
     */
    constexpr expected(const unexpected<E>& unex)
        : value_(Error<E>(unex.error())) {}

    /**
     * @brief Constructs an `expected` object from an `unexpected` error by
     * moving it.
     *
     * @param unex The `unexpected` error to be moved and stored.
     */
    constexpr expected(unexpected<E>&& unex)
        : value_(Error<E>(std::move(unex.error()))) {}

    // Observers

    /**
     * @brief Checks if the `expected` object contains a valid value.
     *
     * @return `true` if it contains a value, `false` if it contains an error.
     */
    [[nodiscard]] constexpr bool has_value() const noexcept {
        return std::holds_alternative<std::monostate>(value_);
    }

    /**
     * @brief Retrieves the stored value.
     *
     * Since the value type is `void`, this function does nothing but can throw
     * if an error exists.
     *
     * @throws std::logic_error If the `expected` contains an error.
     */
    constexpr void value() const {
        if (!has_value()) {
            throw std::logic_error(
                "Attempted to access value, but it contains an error.");
        }
    }

    /**
     * @brief Retrieves a constant reference to the stored error.
     *
     * @return A constant reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr const Error<E>& error() const& {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves a reference to the stored error.
     *
     * @return A reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr Error<E>& error() & {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(value_);
    }

    /**
     * @brief Retrieves an rvalue reference to the stored error.
     *
     * @return An rvalue reference to the stored error.
     * @throws std::logic_error If the `expected` contains a value.
     */
    [[nodiscard]] constexpr Error<E>&& error() && {
        if (has_value()) {
            throw std::logic_error(
                "Attempted to access error, but it contains a value.");
        }
        return std::get<Error<E>>(std::move(value_));
    }

    /**
     * @brief Conversion operator to `bool`.
     *
     * @return `true` if the `expected` contains a value, `false` otherwise.
     */
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // Monadic operations

    /**
     * @brief Applies a function to the `expected` object if it contains a
     * value.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(Func&& func) & -> decltype(func()) {
        if (has_value()) {
            return func();
        }
        return decltype(func())(error());
    }

    /**
     * @brief Applies a constant function to the `expected` object if it
     * contains a value.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(Func&& func) const& -> decltype(func()) {
        if (has_value()) {
            return func();
        }
        return decltype(func())(error());
    }

    /**
     * @brief Applies a function to the `expected` object if it contains a
     * value, moving the error.
     *
     * @tparam Func The type of the function to apply.
     * @param func The function to apply.
     * @return The result of the function if a value exists, or an `expected`
     * containing the error.
     */
    template <typename Func>
    constexpr auto and_then(Func&& func) && -> decltype(func()) {
        if (has_value()) {
            return func();
        }
        return decltype(func())(std::move(error()));
    }

    /**
     * @brief Transforms the stored error using the provided function.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(
        Func&& func) & -> expected<void, decltype(func(std::declval<E&>()))> {
        if (has_value()) {
            return *this;
        }
        return expected<void, decltype(func(std::declval<E&>()))>(
            func(error().error()));
    }

    /**
     * @brief Transforms the stored error using the provided constant function.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(Func&& func)
        const& -> expected<void, decltype(func(std::declval<const E&>()))> {
        if (has_value()) {
            return *this;
        }
        return expected<void, decltype(func(std::declval<const E&>()))>(
            func(error().error()));
    }

    /**
     * @brief Transforms the stored error using the provided function, moving
     * the error.
     *
     * @tparam Func The type of the function to apply to the error.
     * @param func The function to apply to the stored error.
     * @return An `expected` with the transformed error type if an error exists,
     * otherwise the original `expected`.
     */
    template <typename Func>
    constexpr auto transform_error(
        Func&& func) && -> expected<void, decltype(func(std::declval<E&&>()))> {
        if (has_value()) {
            return std::move(*this);
        }
        return expected<void, decltype(func(std::declval<E&&>()))>(
            func(std::move(error().error())));
    }

    // Equality operators

    /**
     * @brief Compares two `expected<void, E>` objects for equality.
     *
     * @param lhs The left-hand side `expected` object.
     * @param rhs The right-hand side `expected` object.
     * @return `true` if both `expected` objects are equal, `false` otherwise.
     */
    friend constexpr bool operator==(const expected& lhs, const expected& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;
        if (lhs.has_value()) {
            return true;
        }
        return lhs.error_ == rhs.error_;
    }

    /**
     * @brief Compares two `expected<void, E>` objects for inequality.
     *
     * @param lhs The left-hand side `expected` object.
     * @param rhs The right-hand side `expected` object.
     * @return `true` if both `expected` objects are not equal, `false`
     * otherwise.
     */
    friend constexpr bool operator!=(const expected& lhs, const expected& rhs) {
        return !(lhs == rhs);
    }

private:
    std::variant<std::monostate, Error<E>>
        value_;  ///< The variant holding either no value or the error.
};

/**
 * @brief Creates an `expected` object containing the given value.
 *
 * @tparam T The type of the value.
 * @param value The value to be stored in the `expected`.
 * @return An `expected` object containing the value.
 */
template <typename T>
constexpr auto make_expected(T&& value) -> expected<std::decay_t<T>> {
    return expected<std::decay_t<T>>(std::forward<T>(value));
}

/**
 * @brief Creates an `unexpected` object containing the given error.
 *
 * @tparam E The type of the error.
 * @param error The error to be stored in the `unexpected`.
 * @return An `unexpected` object containing the error.
 */
template <typename E>
constexpr auto make_unexpected(const E& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(error);
}

/**
 * @brief Creates an `unexpected` object by moving the given error.
 *
 * @tparam E The type of the error.
 * @param error The error to be moved and stored in the `unexpected`.
 * @return An `unexpected` object containing the moved error.
 */
template <typename E>
constexpr auto make_unexpected(E&& error) -> unexpected<std::decay_t<E>> {
    return unexpected<std::decay_t<E>>(std::forward<E>(error));
}

/**
 * @brief Creates an `unexpected` object containing a `std::string` error from a
 * C-style string.
 *
 * @param error The C-style string representing the error.
 * @return An `unexpected<std::string>` object containing the error.
 */
inline auto make_unexpected(const char* error) -> unexpected<std::string> {
    return unexpected<std::string>(std::string(error));
}

}  // namespace atom::type

#endif  // ATOM_TYPE_EXPECTED_HPP