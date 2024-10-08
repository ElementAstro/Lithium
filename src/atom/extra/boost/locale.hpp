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
class LocaleWrapper {
public:
    explicit LocaleWrapper(const std::string& localeName = "") {
        ::boost::locale::generator gen;
        std::locale::global(gen(localeName));
        locale_ = std::locale();
    }

    // String conversion
    static auto toUtf8(const std::string& str,
                       const std::string& fromCharset) -> std::string {
        return ::boost::locale::conv::to_utf<char>(str, fromCharset);
    }

    static auto fromUtf8(const std::string& str,
                         const std::string& toCharset) -> std::string {
        return ::boost::locale::conv::from_utf<char>(str, toCharset);
    }

    // Unicode normalization
    static auto normalize(const std::string& str,
                          ::boost::locale::norm_type norm =
                              ::boost::locale::norm_default) -> std::string {
        return ::boost::locale::normalize(str, norm);
    }

    // Tokenization
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

    // Translation
    static auto translate(const std::string& str, const std::string& /*domain*/,
                          const std::string& localeName = "") -> std::string {
        ::boost::locale::generator gen;
        std::locale loc = gen(localeName);
        return ::boost::locale::translate(str).str(loc);
    }

    // Case conversion
    [[nodiscard]] auto toUpper(const std::string& str) const -> std::string {
        return ::boost::locale::to_upper(str, locale_);
    }

    [[nodiscard]] auto toLower(const std::string& str) const -> std::string {
        return ::boost::locale::to_lower(str, locale_);
    }

    [[nodiscard]] auto toTitle(const std::string& str) const -> std::string {
        return ::boost::locale::to_title(str, locale_);
    }

    // Collation
    [[nodiscard]] auto compare(const std::string& str1,
                               const std::string& str2) const -> int {
        return static_cast<int>(::boost::locale::comparator<
                                char, ::boost::locale::collate_level::primary>(
            locale_)(str1, str2));
    }

    // Date and time formatting
    [[nodiscard]] static auto formatDate(
        const ::boost::posix_time::ptime& dateTime,
        const std::string& format) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << ::boost::locale::format(format) % dateTime;
        return oss.str();
    }

    // Number formatting
    [[nodiscard]] static auto formatNumber(double number,
                                           int precision = 2) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << std::fixed << std::setprecision(precision) << number;
        return oss.str();
    }

    // Currency formatting
    [[nodiscard]] static auto formatCurrency(
        double amount, const std::string& currency) -> std::string {
        std::ostringstream oss;
        oss.imbue(std::locale());
        oss << ::boost::locale::as::currency << currency << amount;
        return oss.str();
    }

    [[nodiscard]] static auto regexReplace(
        const std::string& str, const ::boost::regex& regex,
        const std::string& format) -> std::string {
        return ::boost::regex_replace(
            str, regex, format, ::boost::match_default | ::boost::format_all);
    }

    // Message formatting with named arguments
    template <typename... Args>
    [[nodiscard]] auto format(const std::string& formatString,
                              Args&&... args) const -> std::string {
        return (::boost::locale::format(formatString) % ... % args)
            .str(locale_);
    }

private:
    std::locale locale_;
    static constexpr std::size_t K_BUFFER_SIZE = 4096;
};
}  // namespace atom::extra::boost

#endif
