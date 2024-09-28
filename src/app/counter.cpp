#include "counter.hpp"

#include <algorithm>
#include <fstream>
#include <mutex>

#include "atom/log/loguru.hpp"

void FunctionCounter::startTiming(const std::source_location LOCATION) {
    std::unique_lock lock(mutex);
    auto& stats = counts[LOCATION.function_name()];
    stats.callCount++;
    if (!timeStack.empty()) {
        stats.callers.push_back(timeStack.back().first);
    }
    timeStack.emplace_back(LOCATION.function_name(),
                           std::chrono::high_resolution_clock::now());
    LOG_F(INFO, "Started timing for function: {}", LOCATION.function_name());
}

void FunctionCounter::endTiming() {
    std::unique_lock lock(mutex);
    if (timeStack.empty()) {
        LOG_F(WARNING,
              "End timing called without a corresponding start timing");
        return;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto [func_name, start_time] = timeStack.back();
    timeStack.pop_back();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        endTime - start_time);
    auto& stats = counts[func_name];
    stats.totalTime += duration;
    stats.minTime = std::min(stats.minTime, duration);
    stats.maxTime = std::max(stats.maxTime, duration);

    LOG_F(INFO, "Ended timing for function: {}. Duration: {}", func_name,
          formatDuration(duration));

    if (duration > performanceThreshold) {
        LOG_F(WARNING, "Performance Alert: Function {} took {}", func_name,
              formatDuration(duration));
    }
}

void FunctionCounter::printStats(size_t top_n) {
    std::shared_lock lock(mutex);
    std::vector<std::pair<std::string_view, FunctionStats>> sorted_stats(
        counts.begin(), counts.end());
    std::sort(sorted_stats.begin(), sorted_stats.end(),
              [](const auto& a, const auto& b) {
                  return a.second.callCount > b.second.callCount;
              });

    if (top_n > 0 && top_n < sorted_stats.size()) {
        sorted_stats.resize(top_n);
    }

    LOG_F(INFO, "Printing top {} function stats", top_n);
    printStatsHeader();

    for (const auto& [func, stats] : sorted_stats) {
        printFunctionStats(func, stats);
    }
}

void FunctionCounter::resetStats() {
    std::unique_lock lock(mutex);
    counts.clear();
    timeStack.clear();
    LOG_F(INFO, "Function stats reset");
}

void FunctionCounter::saveStats(const std::string& filename) {
    std::shared_lock lock(mutex);
    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for writing: {}", filename);
        return;
    }

    LOG_F(INFO, "Saving function stats to file: {}", filename);
    for (const auto& [func, stats] : counts) {
        file << func << "," << stats.callCount << "," << stats.totalTime.count()
             << "," << stats.minTime.count() << "," << stats.maxTime.count();
        for (const auto& caller : stats.callers) {
            file << "," << caller;
        }
        file << "\n";
    }
}

void FunctionCounter::loadStats(const std::string& filename) {
    std::unique_lock lock(mutex);
    std::ifstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for reading: {}", filename);
        return;
    }

    counts.clear();
    std::string line;
    LOG_F(INFO, "Loading function stats from file: {}", filename);
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string funcName;
        FunctionStats stats;
        long long totalTime;
        long long minTime;
        long long maxTime;

        if (std::getline(iss, funcName, ',') && iss >> stats.callCount &&
            iss.ignore() && iss >> totalTime && iss.ignore() &&
            iss >> minTime && iss.ignore() && iss >> maxTime) {
            stats.totalTime = std::chrono::nanoseconds(totalTime);
            stats.minTime = std::chrono::nanoseconds(minTime);
            stats.maxTime = std::chrono::nanoseconds(maxTime);

            std::string caller;
            while (std::getline(iss, caller, ',')) {
                stats.callers.push_back(caller);
            }

            counts[funcName] = stats;
            LOG_F(INFO, "Loaded stats for function: {}", funcName);
        }
    }
}

void FunctionCounter::setPerformanceThreshold(
    std::chrono::nanoseconds threshold) {
    std::unique_lock lock(mutex);
    performanceThreshold = threshold;
    LOG_F(INFO, "Set performance threshold to {}", formatDuration(threshold));
}

void FunctionCounter::printCallGraph() {
    std::shared_lock lock(mutex);
    LOG_F(INFO, "Printing Call Graph");
    for (const auto& [func, stats] : counts) {
        LOG_F(INFO, "Function: {}", func);
        for (const auto& caller : stats.callers) {
            LOG_F(INFO, "  Caller: {}", caller);
        }
    }
}

void FunctionCounter::printStatsHeader() {
    LOG_F(INFO,
          std::format("{:<30}{:>10}{:>15}{:>15}{:>15}{:>15}\n", "Function Name",
                      "Calls", "Total Time", "Avg Time", "Min Time", "Max Time")
              .c_str());
}

void FunctionCounter::printFunctionStats(std::string_view func,
                                         const FunctionStats& stats) {
    std::chrono::nanoseconds avgTime{0};
    if (stats.callCount > 0) {
        avgTime =
            std::chrono::nanoseconds(stats.totalTime.count() / stats.callCount);
    }

    LOG_F(INFO,
          std::format("{:<30}{:>10}{:>15}{:>15}{:>15}{:>15}\n", func,
                      stats.callCount, formatDuration(stats.totalTime),
                      formatDuration(avgTime), formatDuration(stats.minTime),
                      formatDuration(stats.maxTime))
              .c_str());
}

auto FunctionCounter::formatDuration(std::chrono::nanoseconds ns)
    -> std::string {
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);
    if (us.count() < 1000) {
        return std::to_string(us.count()) + "µs";
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    if (ms.count() < 1000) {
        return std::to_string(ms.count()) + "ms";
    }
    auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    return std::to_string(s.count()) + "s";
}
