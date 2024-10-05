#ifndef ATOM_EXTRA_INICPP_COMMON_HPP
#define ATOM_EXTRA_INICPP_COMMON_HPP

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>

#include "macro.hpp"

namespace inicpp {

/**
 * @brief Returns a string view of whitespace characters.
 * @return A string view containing whitespace characters.
 */
ATOM_CONSTEXPR auto whitespaces() -> std::string_view { return " \t\n\r\f\v"; }

/**
 * @brief Returns a string view of indent characters.
 * @return A string view containing indent characters.
 */
ATOM_CONSTEXPR auto indents() -> std::string_view { return " \t"; }

/**
 * @brief Trims leading and trailing whitespace from a string.
 * @param str The string to trim.
 */
ATOM_INLINE void trim(std::string &str) {
    auto first = str.find_first_not_of(whitespaces());
    auto last = str.find_last_not_of(whitespaces());

    if (first == std::string::npos || last == std::string::npos) {
        str.clear();
    } else {
        str = str.substr(first, last - first + 1);
    }
}

/**
 * @brief Converts a string view to a long integer.
 * @param value The string view to convert.
 * @return An optional containing the converted long integer, or std::nullopt if
 * conversion fails.
 */
ATOM_INLINE auto strToLong(std::string_view value) -> std::optional<long> {
    long result;
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc()) {
        return result;
    }
    return std::nullopt;
}

/**
 * @brief Converts a string view to an unsigned long integer.
 * @param value The string view to convert.
 * @return An optional containing the converted unsigned long integer, or
 * std::nullopt if conversion fails.
 */
ATOM_INLINE auto strToULong(std::string_view value)
    -> std::optional<unsigned long> {
    unsigned long result;
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc()) {
        return result;
    }
    return std::nullopt;
}

/**
 * @struct StringInsensitiveLess
 * @brief A comparator for case-insensitive string comparison.
 */
struct StringInsensitiveLess {
    /**
     * @brief Compares two strings in a case-insensitive manner.
     * @param lhs The left-hand side string view.
     * @param rhs The right-hand side string view.
     * @return True if lhs is less than rhs, false otherwise.
     */
    auto operator()(std::string_view lhs, std::string_view rhs) const -> bool {
        auto tolower = [](unsigned char ctx) { return std::tolower(ctx); };

        auto lhsRange = std::ranges::subrange(lhs.begin(), lhs.end());
        auto rhsRange = std::ranges::subrange(rhs.begin(), rhs.end());

        return std::ranges::lexicographical_compare(
            lhsRange, rhsRange,
            [tolower](unsigned char first, unsigned char second) {
                return tolower(first) < tolower(second);
            });
    }
};

}  // namespace inicpp

#endif  // ATOM_EXTRA_INICPP_COMMON_HPP