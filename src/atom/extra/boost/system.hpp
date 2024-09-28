#ifndef ATOM_EXTRA_BOOST_SYSTEM_HPP
#define ATOM_EXTRA_BOOST_SYSTEM_HPP

#include <boost/system/error_category.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

namespace atom::extra::boost {

class Error {
public:
    Error() noexcept = default;
    explicit Error(const boost::system::error_code& error_code) noexcept
        : m_ec(error_code) {}
    Error(int error_value,
          const boost::system::error_category& error_category) noexcept
        : m_ec(error_value, error_category) {}

    [[nodiscard]] auto value() const noexcept -> int { return m_ec.value(); }
    [[nodiscard]] auto category() const noexcept
        -> const boost::system::error_category& {
        return m_ec.category();
    }
    [[nodiscard]] auto message() const -> std::string { return m_ec.message(); }
    [[nodiscard]] explicit operator bool() const noexcept {
        return static_cast<bool>(m_ec);
    }

    [[nodiscard]] auto to_boost_error_code() const noexcept
        -> boost::system::error_code {
        return m_ec;
    }

    [[nodiscard]] auto operator==(const Error& other) const noexcept -> bool {
        return m_ec == other.m_ec;
    }

    [[nodiscard]] auto operator!=(const Error& other) const noexcept -> bool {
        return !(*this == other);
    }

private:
    boost::system::error_code m_ec;
};

class Exception : public std::system_error {
public:
    explicit Exception(const Error& error)
        : std::system_error(error.value(), error.category(), error.message()) {}

    [[nodiscard]] auto error() const noexcept -> Error {
        return Error(boost::system::error_code(
            code().value(), boost::system::generic_category()));
    }
};

// Result class specialization for non-void types
template <typename T>
class Result {
public:
    using value_type = T;

    explicit Result(T value) : m_value(std::move(value)) {}
    explicit Result(Error error) : m_error(std::move(error)) {}

    [[nodiscard]] auto has_value() const noexcept -> bool { return !m_error; }
    [[nodiscard]] auto value() const& -> const T& {
        if (!has_value()) {
            throw Exception(m_error);
        }
        return *m_value;
    }
    [[nodiscard]] auto value() && -> T&& {
        if (!has_value()) {
            throw Exception(m_error);
        }
        return std::move(*m_value);
    }
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error;
    }
    [[nodiscard]] auto error() && noexcept -> Error {
        return std::move(m_error);
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value();
    }

    template <typename U>
    auto value_or(U&& default_value) const& -> T {
        return has_value() ? value()
                           : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename F>
    auto map(F&& func) const -> Result<std::invoke_result_t<F, T>> {
        if (has_value()) {
            return Result<std::invoke_result_t<F, T>>(func(*m_value));
        }
        return Result<std::invoke_result_t<F, T>>(Error(m_error));
    }

    template <typename F>
    auto and_then(F&& func) const -> std::invoke_result_t<F, T> {
        if (has_value()) {
            return func(*m_value);
        }
        return std::invoke_result_t<F, T>(Error(m_error));
    }

private:
    std::optional<T> m_value;
    Error m_error;
};

// Result class specialization for void type
template <>
class Result<void> {
public:
    Result() = default;
    explicit Result(Error error) : m_error(std::move(error)) {}

    [[nodiscard]] auto has_value() const noexcept -> bool { return !m_error; }
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error;
    }
    [[nodiscard]] auto error() && noexcept -> Error {
        return std::move(m_error);
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value();
    }

private:
    Error m_error;
};

template <typename F>
auto make_result(F&& func) -> Result<std::invoke_result_t<F>> {
    using return_type = std::invoke_result_t<F>;
    try {
        return Result<return_type>(func());
    } catch (const Exception& e) {
        return Result<return_type>(e.error());
    } catch (const std::exception&) {
        return Result<return_type>(Error(boost::system::errc::invalid_argument,
                                         boost::system::generic_category()));
    }
}

enum class CustomError { ErrorOne = 1, ErrorTwo = 2 };

class CustomCategory : public boost::system::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override {
        return "custom_category";
    }
    [[nodiscard]] auto message(int error_value) const -> std::string override {
        switch (static_cast<CustomError>(error_value)) {
            case CustomError::ErrorOne:
                return "Error One occurred";
            case CustomError::ErrorTwo:
                return "Error Two occurred";
            default:
                return "Unknown error";
        }
    }
};

[[nodiscard]] auto custom_category() -> const boost::system::error_category& {
    static CustomCategory instance;
    return instance;
}

inline auto make_error_code(CustomError error) -> Error {
    return {static_cast<int>(error), custom_category()};
}

auto divide(int numerator, int denominator) -> wrapper::Result<int> {
    if (denominator == 0) {
        return wrapper::Result<int>(
            wrapper::Error(boost::system::errc::make_error_code(
                boost::system::errc::result_out_of_range)));
    }
    return wrapper::Result<int>(numerator / denominator);
}

auto process_result(int value) -> wrapper::Result<std::string> {
    constexpr int threshold = 10;
    if (value > threshold) {
        return wrapper::Result<std::string>("Big number: " +
                                            std::to_string(value));
    }
    return wrapper::Result<std::string>(
        wrapper::make_error_code(wrapper::CustomError::ErrorOne));
}
}  // namespace atom::extra::boost

#endif  // ATOM_EXTRA_BOOST_SYSTEM_HPP