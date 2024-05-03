/*
 * slice.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-12

Description: Slice

**************************************************/

#ifndef ATOM_UTILS_STRING_SLICE_HPP
#define ATOM_UTILS_STRING_SLICE_HPP

#include <cctype>
#include <string>
#include <string_view>

/**
 * @brief Class representing a slice of a string.
 *
 * This class allows efficient manipulation of substrings without copying the
 * underlying data.
 */
class StringSlice {
public:
    /**
     * @brief Default constructor.
     */
    StringSlice() = default;

    /**
     * @brief Construct a StringSlice from a string_view with specified start
     * and end indices.
     *
     * @param sv The string_view to slice.
     * @param start The start index of the slice.
     * @param end The end index of the slice.
     */
    explicit StringSlice(std::string_view sv, size_t start, size_t end);

    /**
     * @brief Construct a StringSlice from a string_view.
     *
     * @param sv The string_view to slice.
     */
    explicit StringSlice(std::string_view sv);

    /**
     * @brief Construct a StringSlice from a std::string with specified start
     * and end indices.
     *
     * @param s The std::string to slice.
     * @param start The start index of the slice.
     * @param end The end index of the slice.
     */
    explicit StringSlice(const std::string& s);

    /**
     * @brief Construct a StringSlice from a std::string.
     *
     * @param s The std::string to slice.
     */
    explicit StringSlice(const std::string& s, size_t start, size_t end);

    /**
     * @brief Overloaded subscript operator to access characters by index.
     *
     * @param index The index of the character to access.
     * @return The character at the specified index.
     */
    char operator[](size_t index) const;

    /**
     * @brief Overloaded subscript operator to access characters by index.
     *
     * @param index The index of the character to access.
     * @return The character at the specified index.
     */
    StringSlice operator()(size_t start, size_t end) const;

    /**
     * @brief Overloaded pre-increment operator to increment the start index.
     *
     * @return A reference to the modified StringSlice.
     */
    StringSlice& operator++();

    /**
     * @brief Overloaded post-increment operator to increment the start index.
     *
     * @return A copy of the original StringSlice before incrementing.
     */
    StringSlice operator++(int);

    /**
     * @brief Overloaded pre-decrement operator to decrement the start index.
     *
     * @return A reference to the modified StringSlice.
     */
    StringSlice& operator--();

    /**
     * @brief Overloaded post-decrement operator to decrement the start index.
     *
     * @return A copy of the original StringSlice before decrementing.
     */
    StringSlice operator--(int);

    /**
     * @brief Overloaded addition operator to add a number to the start index.
     *
     * @param n The number to add.
     * @return A reference to the modified StringSlice.
     */
    StringSlice& operator+=(size_t n);

    /**
     * @brief Overloaded subtraction operator to subtract a number from the
     * start index.
     *
     * @param n The number to subtract.
     * @return A reference to the modified StringSlice.
     */
    StringSlice& operator-=(size_t n);

    /**
     * @brief Overloaded addition operator to add a number to the start index.
     *
     * @param n The number to add.
     * @return A copy of the original StringSlice with the start index
     * incremented by n.
     */
    StringSlice operator+(size_t n) const;

    /**
     * @brief Overloaded subtraction operator to subtract a number from the
     * start index.
     *
     * @param n The number to subtract.
     * @return A copy of the original StringSlice with the start index
     * decremented by n.
     */
    StringSlice operator-(size_t n) const;

    /**
     * @brief Conversion operator to convert the StringSlice to a std::string.
     *
     * @return The StringSlice as a std::string.
     */
    explicit operator std::string() const;

    /**
     * @brief Returns the size of the StringSlice.
     *
     * @return The size of the StringSlice.
     */
    size_t size() const;

    /**
     * @brief Returns whether the StringSlice is empty.
     *
     * @return True if the StringSlice is empty, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Returns the first character of the StringSlice.
     *
     * @return The first character of the StringSlice.
     */
    char front() const;

    /**
     * @brief Returns the last character of the StringSlice.
     *
     * @return The last character of the StringSlice.
     */
    char back() const;

    /**
     * @brief Removes the first n characters from the StringSlice.
     *
     * @param n The number of characters to remove.
     */
    void remove_prefix(size_t n);

    /**
     * @brief Removes the last n characters from the StringSlice.
     *
     * @param n The number of characters to remove.
     */
    void remove_suffix(size_t n);

    /**
     * @brief Swaps the contents of the StringSlice with another StringSlice.
     *
     * @param other The other StringSlice to swap with.
     */
    void swap(StringSlice& other);

    /**
     * @brief Removes leading whitespace from the StringSlice.
     *
     * @return A reference to the modified StringSlice.
     */
    StringSlice& ltrim();

    /**
     * @brief Removes trailing whitespace from the StringSlice.
     *
     * @return A reference to the modified StringSlice.
     */
    StringSlice& rtrim();

    /**
     * @brief Removes leading and trailing whitespace from the StringSlice.
     *
     * @return A reference to the modified StringSlice.
     */
    StringSlice& trim();

    /**
     * @brief Returns a substring of the StringSlice.
     *
     * @param pos The starting position of the substring.
     * @param count The number of characters in the substring.
     * @return The substring.
     */
    StringSlice substr(size_t pos = 0, size_t count = std::string::npos) const;

    /**
     * @brief Compares the StringSlice with another string.
     *
     * @param other The other string to compare with.
     * @return True if the StringSlice is equal to the other string, false
     * otherwise.
     */
    bool equal(std::string_view other) const;

    /**
     * @brief Compares a substring of the StringSlice with another string.
     *
     * @param pos The starting position of the substring.
     * @param count The number of characters in the substring.
     * @param other The other string to compare with.
     * @return True if the substring is equal to the other string, false
     * otherwise.
     */
    bool equal(size_t pos, size_t count, std::string_view other) const;

    /**
     * @brief Checks if the StringSlice starts with a given prefix.
     *
     * @param prefix The prefix to check for.
     * @return True if the StringSlice starts with the given prefix, false
     * otherwise.
     */
    bool starts_with(std::string_view prefix) const;

    /**
     * @brief Checks if the StringSlice ends with a given suffix.
     *
     * @param suffix The suffix to check for.
     * @return True if the StringSlice ends with the given suffix, false
     * otherwise.
     */
    bool ends_with(std::string_view suffix) const;

    /**
     * @brief Finds the first occurrence of a substring in the StringSlice.
     *
     * @param target The substring to search for.
     * @param pos The starting position to search from.
     * @return The position of the first occurrence of the substring, or
     * std::string::npos if not found.
     */
    size_t find(std::string_view target, size_t pos = 0) const;

    /**
     * @brief Finds the last occurrence of a substring in the StringSlice.
     *
     * @param target The substring to search for.
     * @param pos The starting position to search from.
     * @return The position of the last occurrence of the substring, or
     * std::string::npos if not found.
     */
    size_t rfind(std::string_view target, size_t pos = std::string::npos) const;

    std::string_view sv;
    size_t start;
    size_t end;
};

std::ostream& operator<<(std::ostream& os, const StringSlice& slice);

bool operator==(const StringSlice& lhs, const StringSlice& rhs);

bool operator==(const StringSlice& lhs, const std::string& rhs);

bool operator==(const StringSlice& lhs, std::string_view rhs);

bool operator==(const StringSlice& lhs, const char* rhs);

bool operator!=(const StringSlice& lhs, const StringSlice& rhs);

bool operator<=(const StringSlice& lhs, const StringSlice& rhs);

bool operator<(const StringSlice& lhs, const StringSlice& rhs);

bool operator>=(const StringSlice& lhs, const StringSlice& rhs);

bool operator>(const StringSlice& lhs, const StringSlice& rhs);

StringSlice operator+(const StringSlice& lhs, const StringSlice& rhs);

StringSlice operator+(const StringSlice& lhs, const std::string& rhs);

StringSlice operator+(const StringSlice& lhs, std::string_view rhs);

StringSlice operator+(const std::string& lhs, const StringSlice& rhs);

StringSlice operator+(std::string_view lhs, const StringSlice& rhs);

StringSlice operator+(const StringSlice& lhs, const char* rhs);

StringSlice operator+(const char* lhs, const StringSlice& rhs);

#endif