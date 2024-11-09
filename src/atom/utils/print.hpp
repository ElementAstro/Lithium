#ifndef ATOM_UTILS_PRINT_HPP
#define ATOM_UTILS_PRINT_HPP

#include <array>
#include <chrono>
#include <cmath>
#include <deque>
#include <format>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "atom/utils/time.hpp"

namespace atom::utils {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

constexpr int DEFAULT_BAR_WIDTH = 50;
constexpr int PERCENTAGE_MULTIPLIER = 100;
constexpr int SLEEP_DURATION_MS = 200;
constexpr int MAX_LABEL_WIDTH = 15;
constexpr int BUFFER1_SIZE = 1024;
constexpr int BUFFER2_SIZE = 2048;
constexpr int BUFFER3_SIZE = 4096;
constexpr int THREAD_ID_WIDTH = 16;

template <typename Stream, typename... Args>
inline void log(Stream& stream, LogLevel level, std::string_view fmt,
                Args&&... args) {
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
    }

    std::thread::id thisId = std::this_thread::get_id();

    std::hash<std::thread::id> hasher;
    size_t hashValue = hasher(thisId);

    std::ostringstream oss;
    oss << std::hex << std::setw(THREAD_ID_WIDTH) << std::setfill('0')
        << hashValue;
    std::string idHexStr = oss.str();

    stream << "[" << atom::utils::getChinaTimestampString() << "] [" << levelStr
           << "] [" << idHexStr << "] "
           << std::vformat(fmt,
                           std::make_format_args(std::forward<Args>(args)...))
           << std::endl;
}

template <typename Stream, typename... Args>
inline void printToStream(Stream& stream, std::string_view fmt,
                          Args&&... args) {
    stream << std::vformat(fmt,
                           std::make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
inline void print(std::string_view fmt, Args&&... args) {
    printToStream(std::cout, fmt, std::forward<Args>(args)...);
}

template <typename Stream, typename... Args>
inline void printlnToStream(Stream& stream, std::string_view fmt,
                            Args&&... args) {
    printToStream(stream, fmt, std::forward<Args>(args)...);
    stream << std::endl;
}

template <typename... Args>
inline void println(std::string_view fmt, Args&&... args) {
    printlnToStream(std::cout, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void printToFile(const std::string& fileName, std::string_view fmt,
                        Args&&... args) {
    std::ofstream file(fileName, std::ios::app);
    if (file.is_open()) {
        printToStream(file, fmt, std::forward<Args>(args)...);
        file.close();
    } else {
        std::cerr << "Error opening file: " << fileName << std::endl;
    }
}

enum class Color {
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
};

template <typename... Args>
inline void printColored(Color color, std::string_view fmt, Args&&... args) {
    std::cout << "\033[" << static_cast<int>(color) << "m"
              << std::vformat(
                     fmt, std::make_format_args(std::forward<Args>(args)...))
              << "\033[0m";  // 恢复默认颜色
}

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
    Timer() : startTime(std::chrono::high_resolution_clock::now()) {}

    void reset() { startTime = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] inline auto elapsed() const -> double {
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(endTime - startTime).count();
    }
};

class CodeBlock {
private:
    int indentLevel = 0;
    static constexpr int spacesPerIndent = 4;

public:
    constexpr void increaseIndent() { ++indentLevel; }
    constexpr void decreaseIndent() {
        if (indentLevel > 0) {
            --indentLevel;
        }
    }

    template <typename... Args>
    inline void print(std::string_view fmt, Args&&... args) const {
        std::cout << std::string(
            static_cast<size_t>(indentLevel) * spacesPerIndent, ' ');
        atom::utils::print(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void println(std::string_view fmt, Args&&... args) const {
        std::cout << std::string(
            static_cast<size_t>(indentLevel) * spacesPerIndent, ' ');
        atom::utils::println(fmt, std::forward<Args>(args)...);
    }
};

enum class TextStyle {
    BOLD = 1,
    UNDERLINE = 4,
    BLINK = 5,
    REVERSE = 7,
    CONCEALED = 8
};

template <typename... Args>
inline void printStyled(TextStyle style, std::string_view fmt, Args&&... args) {
    std::cout << "\033[" << static_cast<int>(style) << "m"
              << std::vformat(
                     fmt, std::make_format_args(std::forward<Args>(args)...))
              << "\033[0m";
}

class MathStats {
public:
    template <typename Container>
    [[nodiscard]] static inline auto mean(const Container& data) -> double {
        return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    }

    template <typename Container>
    [[nodiscard]] static inline auto median(Container data) -> double {
        std::sort(data.begin(), data.end());
        if (data.size() % 2 == 0) {
            return (data[data.size() / 2 - 1] + data[data.size() / 2]) / 2.0;
        } else {
            return data[data.size() / 2];
        }
    }

    template <typename Container>
    [[nodiscard]] static inline auto standardDeviation(const Container& data)
        -> double {
        double meanValue = mean(data);
        double variance =
            std::accumulate(data.begin(), data.end(), 0.0,
                            [meanValue](double acc, double value) {
                                return acc + (value - meanValue) *
                                                 (value - meanValue);
                            }) /
            data.size();
        return std::sqrt(variance);
    }
};

class MemoryTracker {
private:
    std::unordered_map<std::string, size_t> allocations;

public:
    inline void allocate(const std::string& identifier, size_t size) {
        allocations[identifier] = size;
    }

    inline void deallocate(const std::string& identifier) {
        allocations.erase(identifier);
    }

    inline void printUsage() const {
        size_t total = 0;
        for (const auto& [identifier, size] : allocations) {
            println("{}: {} bytes", identifier, size);
            total += size;
        }
        println("Total memory usage: {} bytes", total);
    }
};

class FormatLiteral {
    std::string_view fmt_str_;

public:
    constexpr explicit FormatLiteral(std::string_view format)
        : fmt_str_(format) {}

    template <typename... Args>
    [[nodiscard]] inline auto operator()(Args&&... args) const -> std::string {
        return std::vformat(fmt_str_,
                            std::make_format_args(std::forward<Args>(args)...));
    }
};
}  // namespace atom::utils

constexpr auto operator""_fmt(const char* str, std::size_t len) {
    return atom::utils::FormatLiteral(std::string_view(str, len));
}

#if __cplusplus >= 202302L
namespace std {

template <typename T>
struct formatter<
    T,
    enable_if_t<is_same_v<T, std::vector<typename T::value_type>> ||
                    is_same_v<T, std::list<typename T::value_type>> ||
                    is_same_v<T, std::set<typename T::value_type>> ||
                    is_same_v<T, std::unordered_set<typename T::value_type>> ||
                    is_same_v<T, std::deque<typename T::value_type>> ||
                    is_same_v<T, std::forward_list<typename T::value_type>>,
                char>> : formatter<std::string_view> {
    auto format(const T& container,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '[';
        bool first = true;
        for (const auto& item : container) {
            if (!first) {
                *out++ = ',';
                *out++ = ' ';
            }
            out = std::format_to(out, "{}", item);
            first = false;
        }
        *out++ = ']';
        return out;
    }
};

template <typename T1, typename T2>
struct formatter<std::map<T1, T2>> : formatter<std::string_view> {
    auto format(const std::map<T1, T2>& m,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '{';
        bool first = true;
        for (const auto& [key, value] : m) {
            if (!first) {
                *out++ = ',';
                *out++ = ' ';
            }
            out = std::format_to(out, "{}: {}", key, value);
            first = false;
        }
        *out++ = '}';
        return out;
    }
};

template <typename T1, typename T2>
struct formatter<std::unordered_map<T1, T2>> : formatter<std::string_view> {
    auto format(const std::unordered_map<T1, T2>& m,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '{';
        bool first = true;
        for (const auto& [key, value] : m) {
            if (!first) {
                *out++ = ',';
                *out++ = ' ';
            }
            out = std::format_to(out, "{}: {}", key, value);
            first = false;
        }
        *out++ = '}';
        return out;
    }
};

template <typename T, std::size_t N>
struct formatter<std::array<T, N>> : formatter<std::string_view> {
    auto format(const std::array<T, N>& arr,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '[';
        for (std::size_t i = 0; i < N; ++i) {
            if (i > 0) {
                *out++ = ',';
                *out++ = ' ';
            }
            out = std::format_to(out, "{}", arr[i]);
        }
        *out++ = ']';
        return out;
    }
};

template <typename T1, typename T2>
struct formatter<std::pair<T1, T2>> : formatter<std::string_view> {
    auto format(const std::pair<T1, T2>& p,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '(';
        out = std::format_to(out, "{}", p.first);
        *out++ = ',';
        *out++ = ' ';
        out = std::format_to(out, "{}", p.second);
        *out++ = ')';
        return out;
    }
};

template <typename... Ts>
struct formatter<std::tuple<Ts...>> : formatter<std::string_view> {
    auto format(const std::tuple<Ts...>& tup,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '(';
        std::apply(
            [&](const Ts&... args) {
                std::size_t n = 0;
                ((void)((n++ > 0 ? (out = std::format_to(out, ", {}", args))
                                 : (out = std::format_to(out, "{}", args))),
                        0),
                 ...);
            },
            tup);
        *out++ = ')';
        return out;
    }
};

template <typename... Ts>
struct formatter<std::variant<Ts...>> : formatter<std::string_view> {
    auto format(const std::variant<Ts...>& var,
                format_context& ctx) const -> decltype(ctx.out()) {
        return std::visit(
            [&ctx](const auto& val) -> decltype(ctx.out()) {
                return std::format_to(ctx.out(), "{}", val);
            },
            var);
    }
};

template <typename T>
struct formatter<std::optional<T>> : formatter<std::string_view> {
    auto format(const std::optional<T>& opt,
                format_context& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        if (opt.has_value()) {
            return std::format_to(out, "Optional({})", opt.value());
        } else {
            return std::format_to(out, "Optional()");
        }
    }
};

}  // namespace std
#endif

#endif
