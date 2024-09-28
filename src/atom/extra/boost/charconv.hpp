#ifndef ATOM_EXTRA_BOOST_CHARCONV_HPP
#define ATOM_EXTRA_BOOST_CHARCONV_HPP

#include <array>
#include <boost/charconv.hpp>
#include <charconv>
#include <cmath>
#include <optional>
#include <string>
#include <system_error>
#include <type_traits>

// 常量定义
constexpr int ALIGNMENT = 16;
constexpr int DEFAULT_BASE = 10;
constexpr size_t BUFFER_SIZE = 128;

enum class NumberFormat { GENERAL, SCIENTIFIC, FIXED, HEX };

struct alignas(ALIGNMENT) FormatOptions {
    NumberFormat format = NumberFormat::GENERAL;
    std::optional<int> precision = std::nullopt;
    bool uppercase = false;
    char thousandsSeparator = '\0';
};

class BoostCharConv {
public:
    // 整数到字符串的转换
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

    // 浮点数到字符串的转换
    template <typename T>
    static auto floatToString(T value, const FormatOptions& options = {})
        -> std::string {
        std::array<char, BUFFER_SIZE> buffer{};
        auto format = getFloatFormat(options.format);
        auto result = options.precision
                          ? boost::charconv::to_chars(
                                buffer.data(), buffer.data() + buffer.size(),
                                value, format, *options.precision)
                          : boost::charconv::to_chars(
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

    // 字符串到整数的转换
    template <typename T>
    static auto stringToInt(const std::string& str,
                            int base = DEFAULT_BASE) -> T {
        T value;
        auto result = boost::charconv::from_chars(
            str.data(), str.data() + str.size(), value, base);
        if (result.ec == std::errc() && result.ptr == str.data() + str.size()) {
            return value;
        }
        throw std::runtime_error("String to int conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    // 字符串到浮点数的转换
    template <typename T>
    static auto stringToFloat(const std::string& str) -> T {
        T value;
        auto result = boost::charconv::from_chars(
            str.data(), str.data() + str.size(), value);
        if (result.ec == std::errc() && result.ptr == str.data() + str.size()) {
            return value;
        }
        throw std::runtime_error("String to float conversion failed: " +
                                 std::make_error_code(result.ec).message());
    }

    // 通用的 toString 函数
    template <typename T>
    static auto toString(T value,
                         const FormatOptions& options = {}) -> std::string {
        if constexpr (std::is_integral_v<T>) {
            return intToString(value, DEFAULT_BASE, options);
        } else if constexpr (std::is_floating_point_v<T>) {
            return floatToString(value, options);
        } else {
            static_assert(always_false<T>, "Unsupported type for toString");
        }
    }

    // 通用的 fromString 函数
    template <typename T>
    static auto fromString(const std::string& str,
                           int base = DEFAULT_BASE) -> T {
        if constexpr (std::is_integral_v<T>) {
            return stringToInt<T>(str, base);
        } else if constexpr (std::is_floating_point_v<T>) {
            return stringToFloat<T>(str);
        } else {
            static_assert(always_false<T>, "Unsupported type for fromString");
        }
    }

    // 特殊值处理
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
    static constexpr bool always_false = false;

    static auto getFloatFormat(NumberFormat format)
        -> boost::charconv::chars_format {
        switch (format) {
            case NumberFormat::SCIENTIFIC:
                return boost::charconv::chars_format::scientific;
            case NumberFormat::FIXED:
                return boost::charconv::chars_format::fixed;
            case NumberFormat::HEX:
                return boost::charconv::chars_format::hex;
            default:
                return boost::charconv::chars_format::general;
        }
    }

    static auto getIntegerFormat(NumberFormat format)
        -> boost::charconv::chars_format {
        return (format == NumberFormat::HEX)
                   ? boost::charconv::chars_format::hex
                   : boost::charconv::chars_format::general;
    }

    static auto addThousandsSeparator(const std::string& str,
                                      char separator) -> std::string {
        std::string result;
        int count = 0;
        bool pastDecimalPoint = false;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            if (*it == '.') {
                pastDecimalPoint = true;
            }
            if (!pastDecimalPoint && count > 0 && count % 3 == 0) {
                result.push_back(separator);
            }
            result.push_back(*it);
            count++;
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    static auto toUpper(std::string str) -> std::string {
        for (char& character : str) {
            character = std::toupper(character);
        }
        return str;
    }
};

#endif
