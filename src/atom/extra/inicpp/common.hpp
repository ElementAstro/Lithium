#ifndef ATOM_EXTRA_INICPP_COMMON_HPP
#define ATOM_EXTRA_INICPP_COMMON_HPP

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>

#include "macro.hpp"

namespace inicpp {

ATOM_CONSTEXPR auto whitespaces() -> std::string_view { return " \t\n\r\f\v"; }
ATOM_CONSTEXPR auto indents() -> std::string_view { return " \t"; }

ATOM_INLINE void trim(std::string &str) {
    auto first = str.find_first_not_of(whitespaces());
    auto last = str.find_last_not_of(whitespaces());

    if (first == std::string::npos || last == std::string::npos) {
        str.clear();
    } else {
        str = str.substr(first, last - first + 1);
    }
}

ATOM_INLINE auto strToLong(std::string_view value) -> std::optional<long> {
    long result;
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc()) {
        return result;
    }
    return std::nullopt;
}

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

struct StringInsensitiveLess {
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
