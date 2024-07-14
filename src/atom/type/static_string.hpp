#ifndef ATOM_EXPERIMENT_SSTRING_HPP
#define ATOM_EXPERIMENT_SSTRING_HPP

#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include "macro.hpp"

template <size_t N>
class StaticString {
public:
    using value_type = char;

    ATOM_CONSTEXPR StaticString() ATOM_NOEXCEPT : M_SIZE(0) { data_[0] = '\0'; }

    template <size_t M>
    ATOM_CONSTEXPR explicit StaticString(const char (&str)[M]) ATOM_NOEXCEPT
        : M_SIZE(M - 1) {
        std::copy_n(str, M, data_);
    }

    ATOM_CONSTEXPR StaticString(const char *str) ATOM_NOEXCEPT {
        M_SIZE = std::strlen(str);
        std::copy_n(str, M_SIZE + 1, data_);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto size() const ATOM_NOEXCEPT -> size_t {
        return M_SIZE;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto cStr() const ATOM_NOEXCEPT -> const
        char * {
        return data_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto begin() const ATOM_NOEXCEPT -> const
        char * {
        return data_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto end() const ATOM_NOEXCEPT -> const
        char * {
        return data_ + M_SIZE;
    }

    ATOM_CONSTEXPR auto operator==(const std::string_view &other) const
        ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) == other;
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator==(T &&other) const ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) ==
               std::string_view(std::forward<T>(other));
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator!=(T &&other) const ATOM_NOEXCEPT->bool {
        return !(*this == std::forward<T>(other));
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator<(T &&other) const ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) <
               std::string_view(std::forward<T>(other));
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator<=(T &&other) const ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) <=
               std::string_view(std::forward<T>(other));
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator>(T &&other) const ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) >
               std::string_view(std::forward<T>(other));
    }

    template <typename T>
    ATOM_CONSTEXPR auto operator>=(T &&other) const ATOM_NOEXCEPT->bool {
        return std::string_view(data_, M_SIZE) >=
               std::string_view(std::forward<T>(other));
    }

    ATOM_CONSTEXPR auto operator+=(char c) -> StaticString<N> & {
        if (M_SIZE < N) {
            data_[M_SIZE] = c;
            ++M_SIZE;
            data_[M_SIZE] = '\0';
        }
        return *this;
    }

    ATOM_CONSTEXPR auto operator+(char c) const -> StaticString<N + 1> {
        StaticString<N + 1> result;
        for (size_t i = 0; i < M_SIZE; ++i) {
            result += data_[i];
        }
        result += c;
        return result;
    }

private:
    size_t M_SIZE;
    char data_[N + 1]{};
};

template <size_t N, size_t M>
ATOM_CONSTEXPR auto operator+(const StaticString<N> &lhs,
                              const StaticString<M> &rhs)
    -> StaticString<N + M> {
    StaticString<N + M> result;
    for (size_t i = 0; i < lhs.size(); ++i) {
        result += lhs.cStr()[i];
    }
    for (size_t i = 0; i < rhs.size(); ++i) {
        result += rhs.cStr()[i];
    }
    return result;
}

#endif