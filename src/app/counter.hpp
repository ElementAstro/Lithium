#ifndef FUNCTION_COUNTER_HPP
#define FUNCTION_COUNTER_HPP

#include <chrono>
#include <map>
#include <shared_mutex>
#include <source_location>
#include <string_view>
#include <vector>
#include "macro.hpp"

#define COUNT_AND_TIME_CALL                                       \
    FunctionCounter::startTiming();                               \
    auto counter_guard = std::make_unique<int>(0);                \
    (void)counter_guard;                                          \
    auto timer_end = [](void*) { FunctionCounter::endTiming(); }; \
    std::unique_ptr<void, decltype(timer_end)> timer_guard(       \
        counter_guard.get(), timer_end);

class FunctionCounter {
public:
    struct FunctionStats {
        size_t callCount = 0;
        std::chrono::nanoseconds totalTime{0};
        std::chrono::nanoseconds minTime{std::chrono::nanoseconds::max()};
        std::chrono::nanoseconds maxTime{std::chrono::nanoseconds::min()};
        std::vector<std::string_view> callers;
    } ATOM_ALIGNAS(64);

    static void startTiming(
        std::source_location LOCATION = std::source_location::current());
    static void endTiming();
    static void printStats(size_t top_n = 0);
    static void resetStats();
    static void saveStats(const std::string& filename);
    static void loadStats(const std::string& filename);
    static void setPerformanceThreshold(std::chrono::nanoseconds threshold);
    static void printCallGraph();

    template <typename Func>
    static void conditionalCount(bool condition, Func&& func);

private:
    static inline std::map<std::string_view, FunctionStats> counts;
    static inline std::vector<std::pair<
        std::string_view, std::chrono::high_resolution_clock::time_point>>
        timeStack;
    static inline std::shared_mutex mutex;
    static inline std::chrono::nanoseconds performanceThreshold;

    static void printStatsHeader();
    static void printFunctionStats(std::string_view func,
                                   const FunctionStats& stats);
    static auto formatDuration(std::chrono::nanoseconds ns) -> std::string;
};

template <typename Func>
void FunctionCounter::conditionalCount(bool condition, Func&& func) {
    if (condition) {
        COUNT_AND_TIME_CALL;
        func();
    } else {
        func();
    }
}

#endif  // FUNCTION_COUNTER_HPP
