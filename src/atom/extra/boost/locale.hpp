#ifndef ATOM_EXTRA_BOOST_LOCALE_HPP
#define ATOM_EXTRA_BOOST_LOCALE_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/locale.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/locale/generator.hpp>
#include <boost/regex.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace atom::extra::boost {

/**
 * @brief A wrapper class for Boost.Locale functionalities.
 *
 * This class provides various utilities for string conversion, Unicode
 * normalization, tokenization, translation, case conversion, collation, date
 * and time formatting, number formatting, currency formatting, and regex
 * replacement using Boost.Locale.
 */
class LocaleWrapper {
public:
    /**
     * @brief Constructs a LocaleWrapper object with the specified locale.
     * @param localeName The name of the locale to use. If empty, the global
     * locale is used.
     */
    explicit LocaleWrapper(const std::string& localeName = "") {
        ::boost::locale::generator gen;
        std::locale::global(gen(localeName));
        locale_ = std::locale();
    }

    /**
     * @brief Converts a string to UTF-8 encoding.
     * @param str The string to convert.
     * @param fromCharset The original character set of the string.
     * @return The UTF-8 encoded string.
     */
    static auto toUtf8(const std::string& str,
                       const std::string& fromCharset) -> std::string {
        return ::boost::locale::conv::to_utf<char>(str, fromCharset);
    }

    /**
     * @brief Converts a UTF-8 encoded string to another character set.
     * @param str The UTF-8 encoded string to convert.
     * @param toCharset The target character set.
     * @return The converted string.
     */
    static auto fromUtf8(const std::string& str,
                         const std::string& toCharset) -> std::string {
        return ::boost::locale::conv::from_utf<char>(str, toCharset);
    }

    /**
     * @brief Normalizes a Unicode string.
     * @param str The string to normalize.
     * @param norm The normalization form to use (default is NFC).
     * @return The normalized string.
     */
    static auto normalize(const std::string& str,
                          ::boost::locale::norm_type norm =
                              ::boost::locale::norm_default) -> std::string {
        return ::boost::locale::normalize(str, norm);
    }

    /**
     * @brief Tokenizes a string into words.
     * @param str The string to tokenize.
     * @param localeName The name of the locale to use for tokenization.
     * @return A vector of tokens.
     */
    static auto tokenize(const std::string& str,
                         const std::string& localeName = "")
        -> std::vector<std::string> {
        ::boost::locale::generator gen;
        std::locale loc = gen(localeName);
        ::boost::locale::boundary::ssegment_index map(
            ::boost::locale::boundary::word, str.begin(), str.end(), loc);
        std::vector<std::string> tokens;
#pragma unroll
        for (const auto& token : map) {
            tokens.push_back(token.str());
        }
        return tokens;
    }

    /**
     * @brief Translates a string to the specified locale.
     * @param str The string to translate.
     * @param domain The domain for the translation (not used in this
     * implementation).
     * @param localeName The name of the locale to use for translation.
     * @return The translated string.
     */
    static auto translate(const std::string& str, const std::string& /*domain*/,
                          const std::string& localeName = "") -> std::string {
        ::boost::locale::generator gen;
        std::locale loc = gen(localeName);
        return ::boost::locale::translate(str).str(loc);
    }

    /**
     * @brief Converts a string to uppercase.
     * @param str The string to convert.
     * @return The uppercase string.
     */
    [[nodiscard]] auto toUpper(const std::string& str) const -> std::string {
        return ::boost::locale::to_upper(str, locale_);
    }

    /**
     * @brief Converts a string to lowercase.
     * @param str The string to convert.
     * @return The lowercase string.
     */
    [[nodiscard]] auto toLower(const std::string& str) const -> std::string {
        return ::boost::locale::to_lower(str, locale_);
    }

    /**
     * @brief Converts a string to title case.
     * @param str The string to convert.
     * @return The title case string.
     */
    [[nodiscard]] auto toTitle(const std::string& str) const -> std::string {
        return ::boost::locale::to_title(str, locale_);
    }

    /**
     * @brief Compares two strings using locale-specific collation rules.
     * @param str1 The first string to compare.
     * @param str2 The second string to compare.
     * @return An integer less than, equal to, or greater than zero if str1 is
     * found, respectively, to be less than, to match, or be greater than str2.
     */
    [[nodiscard]] auto compare(const std::string& str1,
                               const std::string& str2) const -> int {
        return static_cast<int>(::boost::locale::comparator<
                                char, ::boost::locale::collate_level::primary>(
            locale_)(str1, str2));
    }

    /**
     * @brief Formats a date and time according to the specified format.
     * @param dateTime The date and time to format.
     * @param format The format string.
     * @return The formatted date and time string.
     */
    [[nodiscard]] static auto formatDate(
        const ::boost::posix_time::ptime& dateTime,
        const std::string& format) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << ::boost::locale::format(format) % dateTime;
        return oss.str();
    }

    /**
     * @brief Formats a number with the specified precision.
     * @param number The number to format.
     * @param precision The number of decimal places.
     * @return The formatted number string.
     */
    [[nodiscard]] static auto formatNumber(double number,
                                           int precision = 2) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << std::fixed << std::setprecision(precision) << number;
        return oss.str();
    }

    /**
     * @brief Formats a currency amount.
     * @param amount The amount to format.
     * @param currency The currency code.
     * @return The formatted currency string.
     */
    [[nodiscard]] static auto formatCurrency(
        double amount, const std::string& currency) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << ::boost::locale::as::currency << currency << amount;
        return oss.str();
    }

    /**
     * @brief Replaces occurrences of a regex pattern in a string with a format
     * string.
     * @param str The string to search.
     * @param regex The regex pattern to search for.
     * @param format The format string to replace with.
     * @return The resulting string after replacements.
     */
    [[nodiscard]] static auto regexReplace(
        const std::string& str, const ::boost::regex& regex,
        const std::string& format) -> std::string {
        return ::boost::regex_replace(
            str, regex, format, ::boost::match_default | ::boost::format_all);
    }

    /**
     * @brief Formats a string with named arguments.
     * @tparam Args The types of the arguments.
     * @param formatString The format string.
     * @param args The arguments to format.
     * @return The formatted string.
     */
    template <typename... Args>
    [[nodiscard]] auto format(const std::string& formatString,
                              Args&&... args) const -> std::string {
        return (::boost::locale::format(formatString) % ... % args)
            .str(locale_);
    }

private:
    std::locale locale_;  ///< The locale used for various operations.
    static constexpr std::size_t K_BUFFER_SIZE =
        4096;  ///< Buffer size for internal operations.
};

}  // namespace atom::extra::boost

#endif  // ATOM_EXTRA_BOOST_LOCALE_HPP
