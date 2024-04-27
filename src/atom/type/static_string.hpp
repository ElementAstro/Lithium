/*
 * sstring.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-2

Description: A simple static string class

**************************************************/

#ifndef ATOM_EXPERIMENT_SSTRING_HPP
#define ATOM_EXPERIMENT_SSTRING_HPP

#include <cstring>
#include <string_view>

class Static_String;
template <typename T>
concept Stringable = std::is_convertible_v<T, std::string> ||
                     std::is_convertible_v<T, std::string_view> ||
                     std::is_convertible_v<T, const char *> ||
                     std::is_convertible_v<T, Static_String>;

class Static_String {
public:
    /**
     * @brief Constructs a Static_String object from a string literal.
     *
     * @tparam N Size of the string literal.
     * @param str The string literal.
     */
    template <size_t N>
    constexpr Static_String(const char (&str)[N]) noexcept
        : m_size(N - 1), data(str) {}

    /**
     * @brief Gets the size of the Static_String object.
     *
     * @return Size of the Static_String object.
     */
    constexpr size_t size() const noexcept { return m_size; }

    /**
     * @brief Gets a pointer to the C-style string stored in the Static_String
     * object.
     *
     * @return Pointer to the C-style string.
     */
    constexpr const char *c_str() const noexcept { return data; }

    /**
     * @brief Gets an iterator to the beginning of the Static_String object.
     *
     * @return Iterator to the beginning of the Static_String object.
     */
    constexpr const char *begin() const noexcept { return data; }

    /**
     * @brief Gets an iterator to the end of the Static_String object.
     *
     * @return Iterator to the end of the Static_String object.
     */
    constexpr const char *end() const noexcept { return data + m_size; }

    /**
     * @brief Checks if the Static_String object is equal to the provided
     * std::string_view.
     *
     * @param other The std::string_view to compare with.
     * @return true if the Static_String object is equal to the
     * std::string_view, otherwise false.
     */
    constexpr bool operator==(const std::string_view &other) const noexcept {
        return std::string_view(data, m_size) == other;
    }

    /**
     * @brief Checks if the Static_String object is equal to the provided
     * convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is equal to the convertible
     * type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator==(T &&other) const noexcept {
        return std::string_view(data, m_size) == std::forward<T>(other);
    }

    /**
     * @brief Checks if the Static_String object is not equal to the provided
     * convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is not equal to the convertible
     * type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator!=(T &&other) const noexcept {
        return !(*this == std::forward<T>(other));
    }

    /**
     * @brief Checks if the Static_String object is less than the provided
     * convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is less than the convertible
     * type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator<(T &&other) const noexcept {
        return std::string_view(data, m_size) < std::forward<T>(other);
    }

    /**
     * @brief Checks if the Static_String object is less than or equal to the
     * provided convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is less than or equal to the
     * convertible type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator<=(T &&other) const noexcept {
        return std::string_view(data, m_size) <= std::forward<T>(other);
    }

    /**
     * @brief Checks if the Static_String object is greater than the provided
     * convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is greater than the convertible
     * type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator>(T &&other) const noexcept {
        return std::string_view(data, m_size) > std::forward<T>(other);
    }

    /**
     * @brief Checks if the Static_String object is greater than or equal to the
     * provided convertible type.
     *
     * @tparam T The convertible type.
     * @param other The convertible type to compare with.
     * @return true if the Static_String object is greater than or equal to the
     * convertible type, otherwise false.
     */
    template <typename T>
        requires Stringable<T>
    constexpr bool operator>=(T &&other) const noexcept {
        return std::string_view(data, m_size) >= std::forward<T>(other);
    }

private:
    const size_t m_size; /**< Size of the Static_String object. */
    const char *data;    /**< Pointer to the C-style string data. */
};

#endif
