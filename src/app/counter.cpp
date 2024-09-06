#include "counter.hpp"

#include <fstream>
#include <mutex>

#include "atom/log/loguru.hpp"

void FunctionCounter::start_timing(const std::source_location location) {
    std::unique_lock lock(mutex);
    auto& stats = counts[location.function_name()];
    stats.call_count++;
    if (!time_stack.empty()) {
        stats.callers.push_back(time_stack.back().first);
    }
    time_stack.push_back(
        {location.function_name(), std::chrono::high_resolution_clock::now()});
}

void FunctionCounter::end_timing() {
    std::unique_lock lock(mutex);
    if (time_stack.empty())
        return;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto [func_name, start_time] = time_stack.back();
    time_stack.pop_back();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time);
    auto& stats = counts[func_name];
    stats.total_time += duration;
    stats.min_time = std::min(stats.min_time, duration);
    stats.max_time = std::max(stats.max_time, duration);

    if (duration > performance_threshold) {
        LOG_F(WARNING, "Performance Alert: Function {} took {}", func_name,
              format_duration(duration));
    }
}

void FunctionCounter::print_stats(size_t top_n) {
    std::shared_lock lock(mutex);
    std::vector<std::pair<std::string_view, FunctionStats>> sorted_stats(
        counts.begin(), counts.end());
    std::sort(sorted_stats.begin(), sorted_stats.end(),
              [](const auto& a, const auto& b) {
                  return a.second.call_count > b.second.call_count;
              });

    if (top_n > 0 && top_n < sorted_stats.size()) {
        sorted_stats.resize(top_n);
    }

    print_stats_header();

    for (const auto& [func, stats] : sorted_stats) {
        print_function_stats(func, stats);
    }
}

void FunctionCounter::reset_stats() {
    std::unique_lock lock(mutex);
    counts.clear();
    time_stack.clear();
}

void FunctionCounter::save_stats(const std::string& filename) {
    std::shared_lock lock(mutex);
    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for writing: {}", filename);
        return;
    }

    for (const auto& [func, stats] : counts) {
        file << func << "," << stats.call_count << ","
             << stats.total_time.count() << "," << stats.min_time.count() << ","
             << stats.max_time.count();
        for (const auto& caller : stats.callers) {
            file << "," << caller;
        }
        file << "\n";
    }
}

void FunctionCounter::load_stats(const std::string& filename) {
    std::unique_lock lock(mutex);
    std::ifstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open file for reading: {}", filename);
        return;
    }

    counts.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string func_name;
        FunctionStats stats;
        long long total_time, min_time, max_time;

        if (std::getline(iss, func_name, ',') && iss >> stats.call_count &&
            iss.ignore() && iss >> total_time && iss.ignore() &&
            iss >> min_time && iss.ignore() && iss >> max_time) {
            stats.total_time = std::chrono::nanoseconds(total_time);
            stats.min_time = std::chrono::nanoseconds(min_time);
            stats.max_time = std::chrono::nanoseconds(max_time);

            std::string caller;
            while (std::getline(iss, caller, ',')) {
                stats.callers.push_back(caller);
            }

            counts[func_name] = stats;
        }
    }
}

void FunctionCounter::set_performance_threshold(
    std::chrono::nanoseconds threshold) {
    std::unique_lock lock(mutex);
    performance_threshold = threshold;
}

void FunctionCounter::print_call_graph() {
    std::shared_lock lock(mutex);
    LOG_F(INFO, "Printing Call Graph");
    for (const auto& [func, stats] : counts) {
        LOG_F(INFO, "Function: {}", func);
        for (const auto& caller : stats.callers) {
            LOG_F(INFO, "  Caller: {}", caller);
        }
    }
}

void FunctionCounter::print_stats_header() {
    LOG_F(INFO,
          std::format("{:<30}{:>10}{:>15}{:>15}{:>15}{:>15}\n", "Function Name",
                      "Calls", "Total Time", "Avg Time", "Min Time", "Max Time")
              .c_str());
}

void FunctionCounter::print_function_stats(std::string_view func,
                                           const FunctionStats& stats) {
    std::chrono::nanoseconds avg_time{0};
    if (stats.call_count > 0) {
        avg_time = std::chrono::nanoseconds(stats.total_time.count() /
                                            stats.call_count);
    }

    LOG_F(INFO, std::format("{:<30}{:>10}{:>15}{:>15}{:>15}{:>15}\n", func,
                            stats.call_count, format_duration(stats.total_time),
                            format_duration(avg_time),
                            format_duration(stats.min_time),
                            format_duration(stats.max_time))
                    .c_str());
}

std::string FunctionCounter::format_duration(std::chrono::nanoseconds ns) {
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
