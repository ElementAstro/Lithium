/*
 * static_string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-17

Description: A Static String Implementation

**************************************************/

#ifndef ATOM_EXPERIMENT_SSTRING_HPP
#define ATOM_EXPERIMENT_SSTRING_HPP

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <string>

/**
 * @brief A class representing a static string with a fixed maximum size.
 *
 * @tparam N The maximum size of the string (excluding the null terminator).
 */
template <std::size_t N>
class StaticString {
public:
    using value_type = char;
    using size_type = std::size_t;

    /**
     * @brief Default constructor. Constructs an empty StaticString.
     */
    constexpr StaticString() noexcept : size_(0) { data_[0] = '\0'; }

    /**
     * @brief Constructor accepting a C-style string literal.
     *
     * @param str The C-style string literal to initialize the StaticString
     * with.
     */
    explicit constexpr StaticString(const std::array<char, N + 1>& str) noexcept
        : size_(N) {
        std::copy_n(str.begin(), N + 1, data_.begin());
    }

    /**
     * @brief Constructor accepting a std::string.
     *
     * @param str The std::string to initialize the StaticString with.
     */
    explicit StaticString(const std::string& str) noexcept : size_(str.size()) {
        if (str.size() > N) {
            throw std::runtime_error("String size exceeds StaticString capacity");
        }
        std::copy_n(str.begin(), str.size(), data_.begin());
        data_[str.size()] = '\0';
    }

    /**
     * @brief Returns the size of the string.
     *
     * @return The size of the string.
     */
    [[nodiscard]] constexpr auto size() const noexcept -> size_type {
        return size_;
    }

    /**
     * @brief Returns a pointer to the underlying C-style string.
     *
     * @return A pointer to the underlying C-style string.
     */
    [[nodiscard]] constexpr auto cStr() const noexcept -> const char* {
        return data_.data();
    }

    /**
     * @brief Returns an iterator to the beginning of the string.
     *
     * @return An iterator to the beginning of the string.
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> const char* {
        return data_.data();
    }

    /**
     * @brief Returns an iterator to the end of the string.
     *
     * @return An iterator to the end of the string.
     */
    [[nodiscard]] constexpr auto end() const noexcept -> const char* {
        return data_.data() + size_;
    }

    /**
     * @brief Compares the StaticString with a std::string_view for equality.
     *
     * @param other The std::string_view to compare with.
     * @return True if the strings are equal, false otherwise.
     */
    constexpr auto operator==(std::string_view other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) == other;
    }

    /**
     * @brief Compares the StaticString with another string for equality.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if the strings are equal, false otherwise.
     */
    template <typename T>
    constexpr auto operator==(T&& other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) ==
               std::string_view(std::forward<T>(other));
    }

    /**
     * @brief Compares the StaticString with another string for inequality.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if the strings are not equal, false otherwise.
     */
    template <typename T>
    constexpr auto operator!=(T&& other) const noexcept -> bool {
        return !(*this == std::forward<T>(other));
    }

    /**
     * @brief Compares the StaticString with another string for less-than.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if this string is less than the other string, false
     * otherwise.
     */
    template <typename T>
    constexpr auto operator<(T&& other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) <
               std::string_view(std::forward<T>(other));
    }

    /**
     * @brief Compares the StaticString with another string for less-than or
     * equal.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if this string is less than or equal to the other string,
     * false otherwise.
     */
    template <typename T>
    constexpr auto operator<=(T&& other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) <=
               std::string_view(std::forward<T>(other));
    }

    /**
     * @brief Compares the StaticString with another string for greater-than.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if this string is greater than the other string, false
     * otherwise.
     */
    template <typename T>
    constexpr auto operator>(T&& other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) >
               std::string_view(std::forward<T>(other));
    }

    /**
     * @brief Compares the StaticString with another string for greater-than or
     * equal.
     *
     * @tparam T The type of the other string.
     * @param other The other string to compare with.
     * @return True if this string is greater than or equal to the other string,
     * false otherwise.
     */
    template <typename T>
    constexpr auto operator>=(T&& other) const noexcept -> bool {
        return std::string_view(data_.data(), size_) >=
               std::string_view(std::forward<T>(other));
    }

    /**
     * @brief Appends a character to the end of the StaticString.
     *
     * @param character The character to append.
     * @return A reference to the modified StaticString.
     */
    constexpr auto operator+=(char character) noexcept -> StaticString<N>& {
        if (size_ < N) {
            data_[size_++] = character;
            data_[size_] = '\0';
        }
        return *this;
    }

    /**
     * @brief Concatenates a character to the StaticString, producing a new
     * StaticString.
     *
     * @param character The character to concatenate.
     * @return A new StaticString with the character appended.
     */
    constexpr auto operator+(char character) const noexcept
        -> StaticString<N + 1> {
        StaticString<N + 1> result;
        std::copy(begin(), end(), result.data_.begin());
        result += character;
        return result;
    }

    size_type size_;  ///< The current size of the string.
    std::array<char, N + 1>
        data_{};  ///< The underlying data storage for the string.
};

/**
 * @brief Concatenates two StaticString objects.
 *
 * @tparam N The size of the first StaticString.
 * @tparam M The size of the second StaticString.
 * @param lhs The first StaticString.
 * @param rhs The second StaticString.
 * @return A new StaticString containing the concatenation of the two input
 * strings.
 */
template <std::size_t N, std::size_t M>
constexpr auto operator+(const StaticString<N>& lhs,
                         const StaticString<M>& rhs) noexcept
    -> StaticString<N + M> {
    StaticString<N + M> result;
    std::copy(lhs.begin(), lhs.end(), result.data_.begin());
    std::copy(rhs.begin(), rhs.end(), result.data_.begin() + N);
    result += '\0';
    return result;
}

/**
 * @brief Concatenates a StaticString with a string literal.
 *
 * @tparam N The size of the StaticString.
 * @tparam M The size of the string literal (including the null terminator).
 * @param lhs The StaticString.
 * @param rhs The string literal.
 * @return A new StaticString containing the concatenation of the StaticString
 * and the string literal.
 */
template <std::size_t N, std::size_t M>
constexpr auto operator+(const StaticString<N>& lhs,
                         const std::array<char, M>& rhs) noexcept
    -> StaticString<N + M - 1> {
    StaticString<N + M - 1> result;
    std::copy(lhs.begin(), lhs.end(), result.data_.begin());
    std::copy_n(rhs.begin(), M - 1, result.data_.begin() + N);
    result += '\0';
    return result;
}

#endif  // ATOM_EXPERIMENT_SSTRING_HPP