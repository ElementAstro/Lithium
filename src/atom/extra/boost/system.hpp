#ifndef ATOM_EXTRA_BOOST_SYSTEM_HPP
#define ATOM_EXTRA_BOOST_SYSTEM_HPP

#include <boost/system/error_category.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <optional>
#include <string>
#include <system_error>

namespace atom::extra::boost {

class Error {
public:
    Error() noexcept = default;
    explicit Error(const ::boost::system::error_code& error_code) noexcept
        : m_ec_(error_code) {}
    Error(int error_value,
          const ::boost::system::error_category& error_category) noexcept
        : m_ec_(error_value, error_category) {}

    [[nodiscard]] auto value() const noexcept -> int { return m_ec_.value(); }
    [[nodiscard]] auto category() const noexcept
        -> const ::boost::system::error_category& {
        return m_ec_.category();
    }
    [[nodiscard]] auto message() const -> std::string {
        return m_ec_.message();
    }
    [[nodiscard]] explicit operator bool() const noexcept {
        return static_cast<bool>(m_ec_);
    }

    [[nodiscard]] auto toBoostErrorCode() const noexcept
        -> ::boost::system::error_code {
        return m_ec_;
    }

    [[nodiscard]] auto operator==(const Error& other) const noexcept -> bool {
        return m_ec_ == other.m_ec_;
    }

    [[nodiscard]] auto operator!=(const Error& other) const noexcept -> bool {
        return !(*this == other);
    }

private:
    ::boost::system::error_code m_ec_;
};

class Exception : public std::system_error {
public:
    explicit Exception(const Error& error)
        : std::system_error(error.value(), error.category(), error.message()) {}

    [[nodiscard]] auto error() const noexcept -> Error {
        return Error(::boost::system::error_code(
            code().value(), ::boost::system::generic_category()));
    }
};

// Result class specialization for non-void types
template <typename T>
class Result {
public:
    using value_type = T;

    explicit Result(T value) : m_value_(std::move(value)) {}
    explicit Result(Error error) : m_error_(error) {}

    [[nodiscard]] auto hasValue() const noexcept -> bool { return !m_error_; }
    [[nodiscard]] auto value() const& -> const T& {
        if (!hasValue()) {
            throw Exception(m_error_);
        }
        return *m_value_;
    }
    [[nodiscard]] auto value() && -> T&& {
        if (!hasValue()) {
            throw Exception(m_error_);
        }
        return std::move(*m_value_);
    }
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error_;
    }
    [[nodiscard]] auto error() && noexcept -> Error { return m_error_; }

    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    template <typename U>
    auto valueOr(U&& default_value) const& -> T {
        return hasValue() ? value()
                          : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename F>
    auto map(F&& func) const -> Result<std::invoke_result_t<F, T>> {
        if (hasValue()) {
            return Result<std::invoke_result_t<F, T>>(func(*m_value_));
        }
        return Result<std::invoke_result_t<F, T>>(Error(m_error_));
    }

    template <typename F>
    auto andThen(F&& func) const -> std::invoke_result_t<F, T> {
        if (hasValue()) {
            return func(*m_value_);
        }
        return std::invoke_result_t<F, T>(Error(m_error_));
    }

private:
    std::optional<T> m_value_;
    Error m_error_;
};

// Result class specialization for void type
template <>
class Result<void> {
public:
    Result() = default;
    explicit Result(Error error) : m_error_(error) {}

    [[nodiscard]] auto hasValue() const noexcept -> bool { return !m_error_; }
    [[nodiscard]] auto error() const& noexcept -> const Error& {
        return m_error_;
    }
    [[nodiscard]] auto error() && noexcept -> Error { return m_error_; }

    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

private:
    Error m_error_;
};

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
