#ifndef ATOM_UTILS_PRINT_HPP
#define ATOM_UTILS_PRINT_HPP

#include <array>
#include <chrono>
#include <cmath>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
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
void log(Stream& stream, LogLevel level, std::string_view fmt, Args&&... args) {
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
           << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

template <typename Stream, typename... Args>
void printToStream(Stream& stream, std::string_view fmt, Args&&... args) {
    stream << std::vformat(fmt, std::make_format_args(args...));
}

template <typename... Args>
void print(std::string_view fmt, Args&&... args) {
    printToStream(std::cout, fmt, std::forward<Args>(args)...);
}

template <typename Stream, typename... Args>
void printlnToStream(Stream& stream, std::string_view fmt, Args&&... args) {
    printToStream(stream, fmt, std::forward<Args>(args)...);
    stream << std::endl;
}

template <typename... Args>
void println(std::string_view fmt, Args&&... args) {
    printlnToStream(std::cout, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void printToFile(const std::string& fileName, std::string_view fmt,
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
void printColored(Color color, std::string_view fmt, Args&&... args) {
    std::cout << "\033[" << static_cast<int>(color) << "m"
              << std::vformat(fmt, std::make_format_args(args...))
              << "\033[0m";  // 恢复默认颜色
}

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
    Timer() : startTime(std::chrono::high_resolution_clock::now()) {}

    void reset() { startTime = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] auto elapsed() const -> double {
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(endTime - startTime).count();
    }
};

class CodeBlock {
private:
    int indentLevel = 0;
    const int spacesPerIndent = 4;

public:
    void increaseIndent() { ++indentLevel; }
    void decreaseIndent() {
        if (indentLevel > 0) {
            --indentLevel;
        }
    }

    template <typename... Args>
    void print(std::string_view fmt, Args&&... args) {
        std::cout << std::string(
            static_cast<size_t>(indentLevel) * spacesPerIndent, ' ');
        atom::utils::print(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void println(std::string_view fmt, Args&&... args) {
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
void printStyled(TextStyle style, std::string_view fmt, Args&&... args) {
    std::cout << "\033[" << static_cast<int>(style) << "m"
              << std::vformat(fmt, std::make_format_args(args...)) << "\033[0m";
}

class MathStats {
public:
    template <typename Container>
    static auto mean(const Container& data) -> double {
        return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    }

    template <typename Container>
    static auto median(Container data) -> double {
        std::sort(data.begin(), data.end());
        if (data.size() % 2 == 0) {
            return (data[data.size() / 2 - 1] + data[data.size() / 2]) / 2.0;
        } else {
            return data[data.size() / 2];
        }
    }

    template <typename Container>
    static auto standardDeviation(const Container& data) -> double {
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
\
class MemoryTracker {
private:
    std::map<std::string, size_t> allocations;

public:
    void allocate(const std::string& identifier, size_t size) {
        allocations[identifier] = size;
    }

    void deallocate(const std::string& identifier) {
        allocations.erase(identifier);
    }

    void printUsage() {
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
    auto operator()(Args&&... args) const -> std::string {
        return std::vformat(fmt_str_, std::make_format_args(args...));
    }
};

constexpr auto operator""_fmt(const char* str, std::size_t len) {
    return FormatLiteral(std::string_view(str, len));
}
}  // namespace atom::utils

#if __cplusplus >= 202302L
template <typename T>
struct std::formatter<
    T, std::enable_if_t<
           std::is_same_v<T, std::vector<typename T::value_type>> ||
               std::is_same_v<T, std::list<typename T::value_type>> ||
               std::is_same_v<T, std::set<typename T::value_type>> ||
               std::is_same_v<T, std::unordered_set<typename T::value_type>>,
           char>> : std::formatter<std::string_view> {
    auto format(const T& container, format_context& ctx) const {
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

template <typename K, typename V>
struct std::formatter<std::map<K, V>> : std::formatter<std::string_view> {
    auto format(const std::map<K, V>& m, format_context& ctx) const {
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

template <typename K, typename V>
struct std::formatter<std::unordered_map<K, V>>
    : std::formatter<std::string_view> {
    auto format(const std::unordered_map<K, V>& m, format_context& ctx) const {
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
struct std::formatter<std::array<T, N>> : std::formatter<std::string_view> {
    auto format(const std::array<T, N>& arr, format_context& ctx) const {
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
struct std::formatter<std::pair<T1, T2>> : std::formatter<std::string_view> {
    auto format(const std::pair<T1, T2>& p, format_context& ctx) const {
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
#endif

#endif
