#ifndef ATOM_UTILS_PRINT_HPP
#define ATOM_UTILS_PRINT_HPP

#include <chrono>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string_view>
#include <thread>
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

// 打印函数，支持自定义流
template <typename Stream, typename... Args>
void printToStream(Stream& stream, std::string_view fmt, Args&&... args) {
    stream << std::vformat(fmt, std::make_format_args(args...));
}

// 修改后的 print 仅输出到 std::cout
template <typename... Args>
void print(std::string_view fmt, Args&&... args) {
    printToStream(std::cout, fmt, std::forward<Args>(args)...);
}

// 打印并换行
template <typename Stream, typename... Args>
void printlnToStream(Stream& stream, std::string_view fmt, Args&&... args) {
    printToStream(stream, fmt, std::forward<Args>(args)...);
    stream << std::endl;
}

// 打印并换行到 std::cout
template <typename... Args>
void println(std::string_view fmt, Args&&... args) {
    printlnToStream(std::cout, fmt, std::forward<Args>(args)...);
}

// 打印到文件
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

// 设置颜色输出
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

// 新增：带有自动缩进的代码块打印
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

// 新增：设置文本样式
template <typename... Args>
void printStyled(TextStyle style, std::string_view fmt, Args&&... args) {
    std::cout << "\033[" << static_cast<int>(style) << "m"
              << std::vformat(fmt, std::make_format_args(args...)) << "\033[0m";
}

// 新增：简单的数学统计函数
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

// 新增：简单的内存使用跟踪器
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

}  // namespace atom::utils

#endif