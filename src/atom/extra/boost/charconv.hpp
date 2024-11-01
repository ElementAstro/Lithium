#ifndef ATOM_EXTRA_BOOST_CHARCONV_HPP
#define ATOM_EXTRA_BOOST_CHARCONV_HPP

#include <array>
#include <boost/charconv.hpp>
#include <charconv>
#include <cmath>
#include <optional>
#include <ranges>
#include <string>
#include <system_error>
#include <type_traits>

namespace atom::extra::boost {

// Constants definition
constexpr int ALIGNMENT = 16;
constexpr int DEFAULT_BASE = 10;
constexpr size_t BUFFER_SIZE = 128;

/**
 * @brief Enum class representing different number formats.
 */
enum class NumberFormat { GENERAL, SCIENTIFIC, FIXED, HEX };

/**
 * @brief Struct for specifying format options for number conversion.
 */
struct alignas(ALIGNMENT) FormatOptions {
    NumberFormat format = NumberFormat::GENERAL;  ///< The number format.
    std::optional<int> precision =
        std::nullopt;        ///< The precision for floating-point numbers.
    bool uppercase = false;  ///< Whether to use uppercase letters.
    char thousandsSeparator =
        '\0';  ///< The character to use as a thousands separator.
};

/**
 * @brief Class for converting numbers to and from strings using Boost.CharConv.
 */
class BoostCharConv {
public:
    /**
     * @brief Converts an integer to a string.
     * @tparam T The type of the integer.
     * @param value The integer value to convert.
     * @param base The base for the conversion (default is 10).
     * @param options The format options for the conversion.
     * @return The converted string.
     * @throws std::runtime_error if the conversion fails.
     */
    template <typename T>
    static auto intToString(T value, int base = DEFAULT_BASE,
                            const FormatOptions& options = {}) -> std::string {
        static_assert(std::is_integral_v<T>,
                      "intToString only works with integral types");

        std::array<char, BUFFER_SIZE> buffer{};  // Buffer for conversion
        auto result =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                          base);  // Standard to_chars

        if (result.ec == std::errc()) {
            std::string str(buffer.data(), result.ptr);
            if (options.thousandsSeparator != '\0') {
                str = addThousandsSeparator(str, options.thousandsSeparator);
            }
            return options.uppercase ? toUpper(str) : str;
        }
        throw std::runtime_error("Int to string conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    /**
     * @brief Converts a floating-point number to a string.
     * @tparam T The type of the floating-point number.
     * @param value The floating-point value to convert.
     * @param options The format options for the conversion.
     * @return The converted string.
     * @throws std::runtime_error if the conversion fails.
     */
    template <typename T>
    static auto floatToString(T value, const FormatOptions& options = {})
        -> std::string {
        std::array<char, BUFFER_SIZE> buffer{};
        auto format = getFloatFormat(options.format);
        auto result = options.precision
                          ? ::boost::charconv::to_chars(
                                buffer.data(), buffer.data() + buffer.size(),
                                value, format, *options.precision)
                          : ::boost::charconv::to_chars(
                                buffer.data(), buffer.data() + buffer.size(),
                                value, format);
        if (result.ec == std::errc()) {
            std::string str(buffer.data(), result.ptr);
            if (options.thousandsSeparator != '\0') {
                str = addThousandsSeparator(str, options.thousandsSeparator);
            }
            return options.uppercase ? toUpper(str) : str;
        }
        throw std::runtime_error("Float to string conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    /**
     * @brief Converts a string to an integer.
     * @tparam T The type of the integer.
     * @param str The string to convert.
     * @param base The base for the conversion (default is 10).
     * @return The converted integer.
     * @throws std::runtime_error if the conversion fails.
     */
    template <typename T>
    static auto stringToInt(const std::string& str,
                            int base = DEFAULT_BASE) -> T {
        T value;
        auto result = ::boost::charconv::from_chars(
            str.data(), str.data() + str.size(), value, base);
        if (result.ec == std::errc() && result.ptr == str.data() + str.size()) {
            return value;
        }
        throw std::runtime_error("String to int conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    /**
     * @brief Converts a string to a floating-point number.
     * @tparam T The type of the floating-point number.
     * @param str The string to convert.
     * @return The converted floating-point number.
     * @throws std::runtime_error if the conversion fails.
     */
    template <typename T>
    static auto stringToFloat(const std::string& str) -> T {
        T value;
        auto result = ::boost::charconv::from_chars(
            str.data(), str.data() + str.size(), value);
        if (result.ec == std::errc() && result.ptr == str.data() + str.size()) {
            return value;
        }
        throw std::runtime_error("String to float conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    /**
     * @brief Converts a value to a string using the appropriate conversion
     * function.
     * @tparam T The type of the value.
     * @param value The value to convert.
     * @param options The format options for the conversion.
     * @return The converted string.
     */
    template <typename T>
    static auto toString(T value,
                         const FormatOptions& options = {}) -> std::string {
        if constexpr (std::is_integral_v<T>) {
            return intToString(value, DEFAULT_BASE, options);
        } else if constexpr (std::is_floating_point_v<T>) {
            return floatToString(value, options);
        } else {
            static_assert(ALWAYS_FALSE<T>, "Unsupported type for toString");
        }
    }

    /**
     * @brief Converts a string to a value using the appropriate conversion
     * function.
     * @tparam T The type of the value.
     * @param str The string to convert.
     * @param base The base for the conversion (default is 10).
     * @return The converted value.
     */
    template <typename T>
    static auto fromString(const std::string& str,
                           int base = DEFAULT_BASE) -> T {
        if constexpr (std::is_integral_v<T>) {
            return stringToInt<T>(str, base);
        } else if constexpr (std::is_floating_point_v<T>) {
            return stringToFloat<T>(str);
        } else {
            static_assert(ALWAYS_FALSE<T>, "Unsupported type for fromString");
        }
    }

    /**
     * @brief Converts special floating-point values (NaN, Inf) to strings.
     * @tparam T The type of the floating-point value.
     * @param value The floating-point value to convert.
     * @return The converted string.
     */
    template <typename T>
    static auto specialValueToString(T value) -> std::string {
        if (std::isnan(value)) {
            return "NaN";
        }
        if (std::isinf(value)) {
            return value > 0 ? "Inf" : "-Inf";
        }
        return toString(value);
    }

private:
    template <typename T>
    static constexpr bool ALWAYS_FALSE = false;

    /**
     * @brief Gets the Boost.CharConv format for floating-point numbers.
     * @param format The number format.
     * @return The Boost.CharConv format.
     */
    static auto getFloatFormat(NumberFormat format)
        -> ::boost::charconv::chars_format {
        switch (format) {
            case NumberFormat::SCIENTIFIC:
                return ::boost::charconv::chars_format::scientific;
            case NumberFormat::FIXED:
                return ::boost::charconv::chars_format::fixed;
            case NumberFormat::HEX:
                return ::boost::charconv::chars_format::hex;
            default:
                return ::boost::charconv::chars_format::general;
        }
    }

    /**
     * @brief Gets the Boost.CharConv format for integer numbers.
     * @param format The number format.
     * @return The Boost.CharConv format.
     */
    static auto getIntegerFormat(NumberFormat format)
        -> ::boost::charconv::chars_format {
        return (format == NumberFormat::HEX)
                   ? ::boost::charconv::chars_format::hex
                   : ::boost::charconv::chars_format::general;
    }

    /**
     * @brief Adds a thousands separator to a string.
     * @param str The string to modify.
     * @param separator The character to use as a thousands separator.
     * @return The modified string with thousands separators.
     */
    static auto addThousandsSeparator(const std::string& str,
                                      char separator) -> std::string {
        std::string result;
        int count = 0;
        bool pastDecimalPoint = false;
        for (char it : std::ranges::reverse_view(str)) {
            if (it == '.') {
                pastDecimalPoint = true;
            }
            if (!pastDecimalPoint && count > 0 && count % 3 == 0) {
                result.push_back(separator);
            }
            result.push_back(it);
            count++;
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    /**
     * @brief Converts a string to uppercase.
     * @param str The string to convert.
     * @return The converted uppercase string.
     */
    static auto toUpper(std::string str) -> std::string {
        for (char& character : str) {
            character = std::toupper(character);
        }
        return str;
    }
};

}  // namespace atom::extra::boost

#endif  // ATOM_EXTRA_BOOST_CHARCONV_HPP
