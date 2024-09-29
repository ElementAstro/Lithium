#ifndef ATOM_EXTRA_BOOST_REGEX_HPP
#define ATOM_EXTRA_BOOST_REGEX_HPP

#include <boost/regex.hpp>
#include <chrono>
#include <concepts>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace atom::extra::boost {
class RegexWrapper {
public:
    explicit RegexWrapper(std::string_view pattern,
                          ::boost::regex_constants::syntax_option_type flags =
                              ::boost::regex_constants::normal)
        : regex_(pattern.data(), flags) {}

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto match(const T& str) const -> bool {
        return ::boost::regex_match(std::string_view(str).begin(),
                                    std::string_view(str).end(), regex_);
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto search(const T& str) const -> std::optional<std::string> {
        ::boost::smatch what;
        if (::boost::regex_search(std::string(str), what, regex_)) {
            return what.str();
        }
        return std::nullopt;
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto searchAll(const T& str) const -> std::vector<std::string> {
        std::vector<std::string> results;
        std::string s(str);
        ::boost::sregex_iterator iter(s.begin(), s.end(), regex_);
        ::boost::sregex_iterator end;
        for (; iter != end; ++iter) {
            results.push_back(iter->str());
        }
        return results;
    }

    template <typename T, typename U>
        requires std::convertible_to<T, std::string_view> &&
                     std::convertible_to<U, std::string_view>
    auto replace(const T& str, const U& replacement) const -> std::string {
        return ::boost::regex_replace(std::string(str), regex_,
                                      std::string(replacement));
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto split(const T& str) const -> std::vector<std::string> {
        std::vector<std::string> results;
        std::string s(str);
        ::boost::sregex_token_iterator iter(s.begin(), s.end(), regex_, -1);
        ::boost::sregex_token_iterator end;
        for (; iter != end; ++iter) {
            results.push_back(*iter);
        }
        return results;
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto matchGroups(const T& str) const
        -> std::vector<std::pair<std::string, std::vector<std::string>>> {
        std::vector<std::pair<std::string, std::vector<std::string>>> results;
        ::boost::smatch what;
        std::string s(str);
        std::string::const_iterator start = s.begin();
        std::string::const_iterator end = s.end();
        while (::boost::regex_search(start, end, what, regex_)) {
            std::vector<std::string> groups;
            for (size_t i = 1; i < what.size(); ++i) {
                groups.push_back(what[i].str());
            }
            results.emplace_back(what[0].str(), std::move(groups));
            start = what[0].second;
        }
        return results;
    }

    template <typename T, typename Func>
        requires std::convertible_to<T, std::string_view> &&
                 std::invocable<Func, const ::boost::smatch&>
    void forEachMatch(const T& str, Func&& func) const {
        std::string s(str);
        ::boost::sregex_iterator iter(s.begin(), s.end(), regex_);
        ::boost::sregex_iterator end;
        for (; iter != end; ++iter) {
            func(*iter);
        }
    }

    [[nodiscard]] auto getPattern() const -> std::string {
        return regex_.str();
    }

    void setPattern(std::string_view pattern,
                    ::boost::regex_constants::syntax_option_type flags =
                        ::boost::regex_constants::normal) {
        regex_.assign(pattern.data(), flags);
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto namedCaptures(const T& str) const
        -> std::map<std::string, std::string> {
        std::map<std::string, std::string> result;
        ::boost::smatch what;
        if (::boost::regex_match(std::string(str), what, regex_)) {
            for (size_t i = 1; i <= regex_.mark_count(); ++i) {
                result[std::to_string(i)] = what[i].str();
            }
        }
        return result;
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto isValid(const T& str) const -> bool {
        try {
            ::boost::regex_match(std::string_view(str).begin(),
                                 std::string_view(str).end(), regex_);
            return true;
        } catch (const ::boost::regex_error&) {
            return false;
        }
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto replaceCallback(
        const T& str,
        const std::function<std::string(const ::boost::smatch&)>& callback)
        const -> std::string {
        std::string result = std::string(str);
        ::boost::sregex_iterator iter(result.begin(), result.end(), regex_);
        ::boost::sregex_iterator end;

        std::vector<std::pair<std::string::size_type, std::string>>
            replacements;
        while (iter != end) {
            const ::boost::smatch& match = *iter;
            std::string replacement = callback(match);
            replacements.emplace_back(match.position(), std::move(replacement));
            ++iter;
        }

        for (auto iter = replacements.rbegin(); iter != replacements.rend();
             ++iter) {
            result.replace(iter->first, iter->second.length(), iter->second);
        }

        return result;
    }

    [[nodiscard]] static auto escapeString(const std::string& str)
        -> std::string {
        return ::boost::regex_replace(
            str, ::boost::regex(R"([.^$|()\[\]{}*+?\\])"), R"(\\&)",
            ::boost::regex_constants::match_default |
                ::boost::regex_constants::format_sed);
    }

    template <typename T>
        requires std::convertible_to<T, std::string_view>
    auto benchmarkMatch(const T& str, int iterations = 1000) const
        -> std::chrono::nanoseconds {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            ::boost::regex_match(std::string_view(str).begin(),
                                 std::string_view(str).end(), regex_);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                    start) /
               iterations;
    }

    static auto isValidRegex(const std::string& pattern) -> bool {
        try {
            ::boost::regex test(pattern);
            return true;
        } catch (const ::boost::regex_error&) {
            return false;
        }
    }

private:
    ::boost::regex regex_;
};
}  // namespace atom::extra::boost

#endif
