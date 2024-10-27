#ifndef FUNCTION_COUNTER_HPP
#define FUNCTION_COUNTER_HPP

#include <chrono>
#include <map>
#include <shared_mutex>
#include <source_location>
#include <string_view>
#include <vector>

#include "atom/macro.hpp"

/**
 * @brief Macro to count and time a function call.
 *
 * This macro should be placed at the beginning of the function you want to profile.
 * It will automatically start timing when the function is entered and stop timing when the function exits.
 */
#define COUNT_AND_TIME_CALL                                       \
    FunctionCounter::startTiming();                               \
    auto counter_guard = std::make_unique<int>(0);                \
    (void)counter_guard;                                          \
    auto timer_end = [](void*) { FunctionCounter::endTiming(); }; \
    std::unique_ptr<void, decltype(timer_end)> timer_guard(       \
        counter_guard.get(), timer_end);

/**
 * @brief Class to count and time function calls.
 *
 * This class provides static methods to start and stop timing, print statistics, reset statistics,
 * save and load statistics, set performance thresholds, and print call graphs.
 */
class FunctionCounter {
public:
    /**
     * @brief Struct to hold function statistics.
     */
    struct FunctionStats {
        size_t callCount = 0; ///< Number of times the function was called.
        std::chrono::nanoseconds totalTime{0}; ///< Total time spent in the function.
        std::chrono::nanoseconds minTime{std::chrono::nanoseconds::max()}; ///< Minimum time spent in a single call.
        std::chrono::nanoseconds maxTime{std::chrono::nanoseconds::min()}; ///< Maximum time spent in a single call.
        std::vector<std::string_view> callers; ///< List of callers of the function.
    } ATOM_ALIGNAS(64);

    /**
     * @brief Start timing a function call.
     *
     * @param LOCATION The source location of the function call. Defaults to the current source location.
     */
    static void startTiming(
        std::source_location LOCATION = std::source_location::current());

    /**
     * @brief End timing a function call.
     */
    static void endTiming();

    /**
     * @brief Print the statistics of the top N functions.
     *
     * @param top_n The number of top functions to print. If 0, print all functions.
     */
    static void printStats(size_t top_n = 0);

    /**
     * @brief Reset all function statistics.
     */
    static void resetStats();

    /**
     * @brief Save the function statistics to a file.
     *
     * @param filename The name of the file to save the statistics to.
     */
    static void saveStats(const std::string& filename);

    /**
     * @brief Load the function statistics from a file.
     *
     * @param filename The name of the file to load the statistics from.
     */
    static void loadStats(const std::string& filename);

    /**
     * @brief Set the performance threshold for function calls.
     *
     * @param threshold The performance threshold in nanoseconds.
     */
    static void setPerformanceThreshold(std::chrono::nanoseconds threshold);

    /**
     * @brief Print the call graph of the functions.
     */
    static void printCallGraph();

    /**
     * @brief Conditionally count and time a function call.
     *
     * @tparam Func The type of the function to call.
     * @param condition The condition to check.
     * @param func The function to call.
     */
    template <typename Func>
    static void conditionalCount(bool condition, Func&& func);

private:
    static inline std::map<std::string_view, FunctionStats> counts; ///< Map of function names to statistics.
    static inline std::vector<std::pair<
        std::string_view, std::chrono::high_resolution_clock::time_point>>
        timeStack; ///< Stack of function names and start times.
    static inline std::shared_mutex mutex; ///< Mutex to protect the counts and timeStack.
    static inline std::chrono::nanoseconds performanceThreshold; ///< Performance threshold for function calls.

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
