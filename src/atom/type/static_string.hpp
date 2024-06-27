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

class StaticString;
template <typename T>
concept Stringable = std::is_convertible_v<T, std::string> ||
                     std::is_convertible_v<T, std::string_view> ||
                     std::is_convertible_v<T, const char *> ||
                     std::is_convertible_v<T, StaticString>;

class StaticString {
public:
    /**
     * @brief Constructs a Static_String object from a string literal.
     *
     * @tparam N Size of the string literal.
     * @param str The string literal.
     */
    template <size_t N>
    constexpr explicit StaticString(const char (&str)[N]) noexcept
        : M_SIZE(N - 1), data_(str) {}

    /**
     * @brief Gets the size of the Static_String object.
     *
     * @return Size of the Static_String object.
     */
    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return M_SIZE;
    }

    /**
     * @brief Gets a pointer to the C-style string stored in the Static_String
     * object.
     *
     * @return Pointer to the C-style string.
     */
    [[nodiscard]] constexpr auto cStr() const noexcept -> const char * {
        return data_;
    }

    /**
     * @brief Gets an iterator to the beginning of the Static_String object.
     *
     * @return Iterator to the beginning of the Static_String object.
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> const char * {
        return data_;
    }

    /**
     * @brief Gets an iterator to the end of the Static_String object.
     *
     * @return Iterator to the end of the Static_String object.
     */
    [[nodiscard]] constexpr auto end() const noexcept -> const char * {
        return data_ + M_SIZE;
    }

    /**
     * @brief Checks if the Static_String object is equal to the provided
     * std::string_view.
     *
     * @param other The std::string_view to compare with.
     * @return true if the Static_String object is equal to the
     * std::string_view, otherwise false.
     */
    constexpr auto operator==(const std::string_view &other) const noexcept
        -> bool {
        return std::string_view(data_, M_SIZE) == other;
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
    constexpr auto operator==(T &&other) const noexcept -> bool {
        return std::string_view(data_, M_SIZE) == std::forward<T>(other);
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
    constexpr auto operator!=(T &&other) const noexcept -> bool {
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
    constexpr auto operator<(T &&other) const noexcept -> bool {
        return std::string_view(data_, M_SIZE) < std::forward<T>(other);
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
    constexpr auto operator<=(T &&other) const noexcept -> bool {
        return std::string_view(data_, M_SIZE) <= std::forward<T>(other);
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
    constexpr auto operator>(T &&other) const noexcept -> bool {
        return std::string_view(data_, M_SIZE) > std::forward<T>(other);
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
    constexpr auto operator>=(T &&other) const noexcept -> bool {
        return std::string_view(data_, M_SIZE) >= std::forward<T>(other);
    }

private:
    const size_t M_SIZE; /**< Size of the Static_String object. */
    const char *data_;   /**< Pointer to the C-style string data. */
};

#endif
