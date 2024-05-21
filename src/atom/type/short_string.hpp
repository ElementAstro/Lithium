/*
 * short_string.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-21

Description: ShortString for Atom

**************************************************/

#ifndef ATOM_TYPE_SHORT_STRING_HPP
#define ATOM_TYPE_SHORT_STRING_HPP

#include <string>

namespace atom::type {
/**
 * @brief A class representing a short string with a maximum length of 15
 * characters.
 */
class ShortString {
private:
    static constexpr size_t MAX_LENGTH = 15;  ///< Maximum length of the string
    std::string str;                          ///< The underlying string

public:
    /**
     * @brief Default constructor.
     */
    ShortString() = default;

    /**
     * @brief Constructs a ShortString object from a std::string.
     * @param s The input string.
     */
    explicit ShortString(const std::string& s);

    /**
     * @brief Constructs a ShortString object from a std::string_view.
     * @param s The input string view.
     */
    explicit ShortString(std::string_view s);

    /**
     * @brief Constructs a ShortString object from a C-string.
     * @param s The input C-string.
     */
    ShortString(const char* s);

    /**
     * @brief Copy constructor.
     * @param other The ShortString object to copy from.
     */
    ShortString(const ShortString& other) = default;

    /**
     * @brief Move constructor.
     * @param other The ShortString object to move from.
     */
    ShortString(ShortString&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     * @param other The ShortString object to copy assign from.
     * @return Reference to the assigned ShortString object.
     */
    ShortString& operator=(const ShortString& other) = default;

    /**
     * @brief Move assignment operator.
     * @param other The ShortString object to move assign from.
     * @return Reference to the assigned ShortString object.
     */
    ShortString& operator=(ShortString&& other) noexcept = default;

    /**
     * @brief Assignment operator for std::string.
     * @param s The input string.
     * @return Reference to the assigned ShortString object.
     */
    ShortString& operator=(const std::string& s);

    /**
     * @brief Assignment operator for C-string.
     * @param s The input C-string.
     * @return Reference to the assigned ShortString object.
     */
    ShortString& operator=(const char* s);

    /**
     * @brief Assignment operator for std::string_view.
     * @param s The input string view.
     * @return Reference to the assigned ShortString object.
     */
    ShortString& operator=(std::string_view s);

    /**
     * @brief Overloaded stream insertion operator.
     * @param os The output stream.
     * @param ss The ShortString object to insert into the stream.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const ShortString& ss);

    /**
     * @brief Concatenates two ShortString objects.
     * @param other The ShortString object to concatenate with.
     * @return A new ShortString object containing the concatenated string.
     */
    ShortString operator+(const ShortString& other) const;

    /**
     * @brief Appends another ShortString object to the current one.
     * @param other The ShortString object to append.
     * @return Reference to the modified ShortString object.
     */
    ShortString& operator+=(const ShortString& other);

    /**
     * @brief Appends a std::string_view to the current ShortString object.
     * @param other The string view to append.
     * @return Reference to the modified ShortString object.
     */
    ShortString& operator+=(std::string_view other);

    /**
     * @brief Equality comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if both strings are equal, false otherwise.
     */
    [[nodiscard]] bool operator==(const ShortString& other) const noexcept;

    /**
     * @brief Inequality comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if both strings are not equal, false otherwise.
     */
    [[nodiscard]] bool operator!=(const ShortString& other) const noexcept;

    /**
     * @brief Less than comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if the current string is less than the other string, false
     * otherwise.
     */
    [[nodiscard]] bool operator<(const ShortString& other) const noexcept;

    /**
     * @brief Greater than comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if the current string is greater than the other string,
     * false otherwise.
     */
    [[nodiscard]] bool operator>(const ShortString& other) const noexcept;

    /**
     * @brief Less than or equal to comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if the current string is less than or equal to the other
     * string, false otherwise.
     */
    [[nodiscard]] bool operator<=(const ShortString& other) const noexcept;

    /**
     * @brief Greater than or equal to comparison operator.
     * @param other The ShortString object to compare with.
     * @return True if the current string is greater than or equal to the other
     * string, false otherwise.
     */
    [[nodiscard]] bool operator>=(const ShortString& other) const noexcept;

    /**
     * @brief Accesses a character in the ShortString object.
     * @param index The index of the character to access.
     * @return Reference to the accessed character.
     */
    [[nodiscard]] char& operator[](size_t index) noexcept;

    /**
     * @brief Accesses a character in the ShortString object (const version).
     * @param index The index of the character to access.
     * @return Reference to the accessed character.
     */
    [[nodiscard]] const char& operator[](size_t index) const noexcept;

    /**
     * @brief Gets the length of the ShortString object.
     * @return The length of the string.
     */
    [[nodiscard]] size_t length() const noexcept;

    /**
     * @brief Returns a substring of the ShortString object.
     * @param pos The starting position of the substring.
     * @param count The length of the substring. If npos, the substring includes
     * all characters from pos to the end.
     * @return A new ShortString object containing the substring.
     */
    ShortString substr(size_t pos = 0, size_t count = std::string::npos) const;

    /**
     * @brief Clears the ShortString object, setting it to an empty string.
     */
    void clear() noexcept;

    /**
     * @brief Swaps the contents of this ShortString object with another
     * ShortString object.
     * @param other The other ShortString object to swap with.
     */
    void swap(ShortString& other) noexcept;
};
}  // namespace atom::type

#endif