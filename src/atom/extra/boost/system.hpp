#ifndef ATOM_EXTRA_BOOST_SYSTEM_HPP
#define ATOM_EXTRA_BOOST_SYSTEM_HPP

#if __has_include(<boost/system/error_category.hpp>)
#include <boost/system/error_category.hpp>
#endif
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <optional>
#include <string>
#include <system_error>

namespace atom::extra::boost {

/**
 * @class Error
 * @brief A wrapper class for Boost.System error codes.
 */
class Error {
public:
    /**
     * @brief Default constructor.
     */
    Error() noexcept = default;

    /**
     * @brief Constructs an Error from a Boost.System error code.
     * @param error_code The Boost.System error code.
     */
    explicit Error(const ::boost::system::error_code& error_code) noexcept
        : m_ec_(error_code) {}

    /**
     * @brief Constructs an Error from an error value and category.
     * @param error_value The error value.
     * @param error_category The error category.
     */
    Error(int error_value,
          const ::boost::system::error_category& error_category) noexcept
        : m_ec_(error_value, error_category) {}

    /**
     * @brief Gets the error value.
     * @return The error value.
     */
    [[nodiscard]] auto value() const noexcept -> int { return m_ec_.value(); }

    /**
     * @brief Gets the error category.
     * @return The error category.
     */
    [[nodiscard]] auto category() const noexcept
        -> const ::boost::system::error_category& {
        return m_ec_.category();
    }

    /**
     * @brief Gets the error message.
     * @return The error message.
     */
    [[nodiscard]] auto message() const -> std::string {
        return m_ec_.message();
    }

    /**
     * @brief Checks if the error code is valid.
     * @return True if the error code is valid, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return static_cast<bool>(m_ec_);
    }

    /**
     * @brief Converts to a Boost.System error code.
     * @return The Boost.System error code.
     */
    [[nodiscard]] auto toBoostErrorCode() const noexcept
        -> ::boost::system::error_code {
        return m_ec_;
    }

    /**
     * @brief Equality operator.
     * @param other The other Error to compare.
     * @return True if the errors are equal, false otherwise.
     */
    [[nodiscard]] auto operator==(const Error& other) const noexcept -> bool {
        return m_ec_ == other.m_ec_;
    }

    /**
     * @brief Inequality operator.
     * @param other The other Error to compare.
     * @return True if the errors are not equal, false otherwise.
     */
    [[nodiscard]] auto operator!=(const Error& other) const noexcept -> bool {
        return !(*this == other);
    }

private:
    ::boost::system::error_code m_ec_;  ///< The Boost.System error code.
};

/**
 * @class Exception
 * @brief A custom exception class for handling errors.
 */
class Exception : public std::system_error {
public:
    /**
     * @brief Constructs an Exception from an Error.
     * @param error The Error object.
     */
    explicit Exception(const Error& error)
        : std::system_error(error.value(), error.category(), error.message()) {}

    /**
     * @brief Gets the associated Error.
     * @return The associated Error.
     */
    [[nodiscard]] auto error() const noexcept -> Error {
        return Error(::boost::system::error_code(
            code().value(), ::boost::system::generic_category()));
    }
};

/**
 * @class Result
 * @brief A class template for handling results with potential errors.
 * @tparam T The type of the result value.
 */
template <typename T>
class Result {
public:
    using value_type = T;  ///< The type of the result value.

    /**
     * @brief Constructs a Result with a value.
     * @param value The result value.
     */
    explicit Result(T value) : m_value_(std::move(value)) {}

    /**
     * @brief Constructs a Result with an Error.
     * @param error The Error object.
     */
    explicit Result(Error error) : m_error_(error) {}

    /**
     * @brief Checks if the Result has a value.
     * @return True if the Result has a value, false otherwise.
     */
    [[nodiscard]] auto hasValue() const noexcept -> bool { return !m_error_; }

    /**
     * @brief Gets the result value.
     * @return The result value.
     * @throws Exception if there is an error.
     */
    [[nodiscard]] auto value() const& -> const T& {
        if (!hasValue()) {
            throw Exception(m_error_);
        }
        return *m_value_;
    }

    /**
     * @brief Gets the result value.
     * @return The result value.
     * @throws Exception if there is an error.
     */
    [[nodiscard]] auto value() && -> T&& {
        if (!hasValue()) {
            throw Exception(m_error_);
        }
        return std::move(*m_value_);
    }

    /**
     * @brief Gets the associated Error.
     * @return The associated Error.
     */
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error_;
    }

    /**
     * @brief Gets the associated Error.
     * @return The associated Error.
     */
    [[nodiscard]] auto error() && noexcept -> Error { return m_error_; }

    /**
     * @brief Checks if the Result has a value.
     * @return True if the Result has a value, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    /**
     * @brief Gets the result value or a default value.
     * @tparam U The type of the default value.
     * @param default_value The default value.
     * @return The result value or the default value.
     */
    template <typename U>
    auto valueOr(U&& default_value) const& -> T {
        return hasValue() ? value()
                          : static_cast<T>(std::forward<U>(default_value));
    }

    /**
     * @brief Applies a function to the result value if it exists.
     * @tparam F The type of the function.
     * @param func The function to apply.
     * @return A new Result with the function applied.
     */
    template <typename F>
    auto map(F&& func) const -> Result<std::invoke_result_t<F, T>> {
        if (hasValue()) {
            return Result<std::invoke_result_t<F, T>>(func(*m_value_));
        }
        return Result<std::invoke_result_t<F, T>>(Error(m_error_));
    }

    /**
     * @brief Applies a function to the result value if it exists.
     * @tparam F The type of the function.
     * @param func The function to apply.
     * @return The result of the function.
     */
    template <typename F>
    auto andThen(F&& func) const -> std::invoke_result_t<F, T> {
        if (hasValue()) {
            return func(*m_value_);
        }
        return std::invoke_result_t<F, T>(Error(m_error_));
    }

private:
    std::optional<T> m_value_;  ///< The result value.
    Error m_error_;             ///< The associated Error.
};

/**
 * @class Result<void>
 * @brief A specialization of the Result class for void type.
 */
template <>
class Result<void> {
public:
    /**
     * @brief Default constructor.
     */
    Result() = default;

    /**
     * @brief Constructs a Result with an Error.
     * @param error The Error object.
     */
    explicit Result(Error error) : m_error_(error) {}

    /**
     * @brief Checks if the Result has a value.
     * @return True if the Result has a value, false otherwise.
     */
    [[nodiscard]] auto hasValue() const noexcept -> bool { return !m_error_; }

    /**
     * @brief Gets the associated Error.
     * @return The associated Error.
     */
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error_;
    }

    /**
     * @brief Gets the associated Error.
     * @return The associated Error.
     */
    [[nodiscard]] auto error() && noexcept -> Error { return m_error_; }

    /**
     * @brief Checks if the Result has a value.
     * @return True if the Result has a value, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

private:
    Error m_error_;  ///< The associated Error.
};

/**
 * @brief Creates a Result from a function.
 * @tparam F The type of the function.
 * @param func The function to execute.
 * @return A Result with the function's return value or an Error.
 */
template <typename F>
auto makeResult(F&& func) -> Result<std::invoke_result_t<F>> {
    using return_type = std::invoke_result_t<F>;
    try {
        return Result<return_type>(func());
    } catch (const Exception& e) {
        return Result<return_type>(e.error());
    } catch (const std::exception&) {
        return Result<return_type>(
            Error(::boost::system::errc::invalid_argument,
                  ::boost::system::generic_category()));
    }
}

}  // namespace atom::extra::boost

#endif  // ATOM_EXTRA_BOOST_SYSTEM_HPP
