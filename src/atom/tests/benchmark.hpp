#ifndef ATOM_TESTS_BENCHMARK_HPP
#define ATOM_TESTS_BENCHMARK_HPP

#include <chrono>
#include <cmath>
#include <future>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "atom/atom/macro.hpp"

/**
 * @brief Class for benchmarking code performance.
 */
class Benchmark {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    /**
     * @brief Configuration settings for the benchmark.
     */
    struct Config {
        int minIterations = 10;             ///< Minimum number of iterations.
        double minDurationSec = 1.0;        ///< Minimum duration in seconds.
        bool async = false;                 ///< Run benchmark asynchronously.
        bool warmup = true;                 ///< Perform a warmup run.
        std::string exportFormat = "json";  ///< Format for exporting results.

        Config() {}
    } ATOM_ALIGNAS(64);

    /**
     * @brief Memory usage statistics.
     */
    struct MemoryStats {
        size_t currentUsage;  ///< Current memory usage.
        size_t peakUsage;     ///< Peak memory usage.
    } ATOM_ALIGNAS(16);

    /**
     * @brief CPU usage statistics.
     */
    struct CPUStats {
        long long instructionsExecuted;  ///< Number of instructions executed.
        long long cyclesElapsed;         ///< Number of CPU cycles elapsed.
        long long branchMispredictions;  ///< Number of branch mispredictions.
        long long cacheMisses;           ///< Number of cache misses.
    } ATOM_ALIGNAS(32);

    /**
     * @brief Construct a new Benchmark object.
     *
     * @param suiteName Name of the benchmark suite.
     * @param name Name of the benchmark.
     * @param config Configuration settings for the benchmark.
     */
    Benchmark(std::string suiteName, std::string name, Config config = {});

    /**
     * @brief Run the benchmark with setup, function, and teardown steps.
     *
     * @tparam SetupFunc Type of the setup function.
     * @tparam Func Type of the function to benchmark.
     * @tparam TeardownFunc Type of the teardown function.
     * @param setupFunc Function to set up the benchmark environment.
     * @param func Function to benchmark.
     * @param teardownFunc Function to clean up after the benchmark.
     */
    template <typename SetupFunc, typename Func, typename TeardownFunc>
    void run(SetupFunc&& setupFunc, Func&& func, TeardownFunc&& teardownFunc) {
        Log("Starting benchmark: " + name_);
        auto runBenchmark = [&]() {
            std::vector<Duration> durations;
            std::vector<MemoryStats> memoryStats;
            std::vector<CPUStats> cpuStats;
            std::size_t totalOpCount = 0;

            if (config_.warmup) {
                Log("Warmup run for benchmark: " + name_);
                WarmupRun(setupFunc, func, teardownFunc);
            }

            auto startTime = Clock::now();
            while (durations.size() <
                       static_cast<size_t>(config_.minIterations) ||
                   std::chrono::duration<double>(Clock::now() - startTime)
                           .count() < config_.minDurationSec) {
                Log("Starting iteration for benchmark: " + name_);
                auto setupData = setupFunc();
                auto memStat = getMemoryUsage();
                auto cpuStatStart = getCpuStats();
                TimePoint start = Clock::now();

                totalOpCount += func(setupData);

                durations.push_back(Clock::now() - start);
                auto cpuStatEnd = getCpuStats();
                teardownFunc(setupData);

                memoryStats.push_back(memStat);
                cpuStats.push_back(subtractCpuStats(cpuStatEnd, cpuStatStart));

                Log("Completed iteration for benchmark: " + name_);
            }

            Log("Analyzing results for benchmark: " + name_);
            analyzeResults(durations, memoryStats, cpuStats, totalOpCount);
            Log("Completed benchmark: " + name_);
        };

        if (config_.async) {
            std::future<void> future =
                std::async(std::launch::async, runBenchmark);
            future.get();
        } else {
            runBenchmark();
        }
    }

    /**
     * @brief Print the benchmark results.
     *
     * @param suite Optional suite name to filter results.
     */
    static void printResults(const std::string& suite = "");

    /**
     * @brief Export the benchmark results to a file.
     *
     * @param filename Name of the file to export results to.
     */
    static void exportResults(const std::string& filename);

private:
    /**
     * @brief Structure to hold benchmark results.
     */
    struct Result {
        std::string name;            ///< Name of the benchmark.
        double averageDuration{};    ///< Average duration of the benchmark.
        double minDuration{};        ///< Minimum duration of the benchmark.
        double maxDuration{};        ///< Maximum duration of the benchmark.
        double medianDuration{};     ///< Median duration of the benchmark.
        double standardDeviation{};  ///< Standard deviation of the durations.
        int iterations{};            ///< Number of iterations.
        double throughput{};         ///< Throughput of the benchmark.
        double avgMemoryUsage{};     ///< Average memory usage.
        double peakMemoryUsage{};    ///< Peak memory usage.
        CPUStats avgCPUStats{};      ///< Average CPU statistics.
    } ATOM_ALIGNAS(128);

    /**
     * @brief Perform a warmup run of the benchmark.
     *
     * @param setupFunc Function to set up the benchmark environment.
     * @param func Function to benchmark.
     * @param teardownFunc Function to clean up after the benchmark.
     */
    void warmupRun(const auto& setupFunc, const auto& func,
                   const auto& teardownFunc);

    /**
     * @brief Calculate the total duration from a vector of durations.
     *
     * @param durations Vector of durations.
     * @return Duration Total duration.
     */
    static auto totalDuration(const std::vector<Duration>& durations)
        -> Duration;

    /**
     * @brief Analyze the results of the benchmark.
     *
     * @param durations Vector of durations.
     * @param memoryStats Vector of memory statistics.
     * @param cpuStats Vector of CPU statistics.
     * @param totalOpCount Total number of operations performed.
     */
    void analyzeResults(const std::vector<Duration>& durations,
                        const std::vector<MemoryStats>& memoryStats,
                        const std::vector<CPUStats>& cpuStats,
                        std::size_t totalOpCount);

    /**
     * @brief Calculate the standard deviation of a vector of values.
     *
     * @param values Vector of values.
     * @return double Standard deviation.
     */
    static auto calculateStandardDeviation(const std::vector<double>& values)
        -> double;

    /**
     * @brief Calculate the average CPU statistics from a vector of CPU
     * statistics.
     *
     * @param stats Vector of CPU statistics.
     * @return CPUStats Average CPU statistics.
     */
    static auto calculateAverageCpuStats(const std::vector<CPUStats>& stats)
        -> CPUStats;

    /**
     * @brief Subtract two CPU statistics structures.
     *
     * @param end Ending CPU statistics.
     * @param start Starting CPU statistics.
     * @return CPUStats Difference between the two CPU statistics.
     */
    static auto subtractCpuStats(const CPUStats& end,
                                 const CPUStats& start) -> CPUStats;

    /**
     * @brief Get the current memory usage statistics.
     *
     * @return MemoryStats Current memory usage statistics.
     */
    static auto getMemoryUsage() -> MemoryStats;

    /**
     * @brief Get the current CPU usage statistics.
     *
     * @return CPUStats Current CPU usage statistics.
     */
    static auto getCpuStats() -> CPUStats;

    /**
     * @brief Log a message.
     *
     * @param message Message to log.
     */
    static void Log(const std::string& message);

    std::string suiteName_;  ///< Name of the benchmark suite.
    std::string name_;       ///< Name of the benchmark.
    Config config_;          ///< Configuration settings for the benchmark.
    static std::map<std::string, std::vector<Result>>
        results;                   ///< Map of benchmark results.
    static std::mutex printMutex;  ///< Mutex for printing results.
    static std::mutex logMutex;    ///< Mutex for logging messages.
};

/**
 * @brief Macro to define and run a benchmark.
 *
 * @param suiteName Name of the benchmark suite.
 * @param name Name of the benchmark.
 * @param setupFunc Function to set up the benchmark environment.
 * @param func Function to benchmark.
 * @param teardownFunc Function to clean up after the benchmark.
 * @param config Configuration settings for the benchmark.
 */
#define BENCHMARK(suiteName, name, setupFunc, func, teardownFunc, config) \
    Benchmark(suiteName, name, config).Run(setupFunc, func, teardownFunc)

#endif  // ATOM_TESTS_BENCHMARK_HPP
