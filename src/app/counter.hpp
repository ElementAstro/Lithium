#ifndef FUNCTION_COUNTER_HPP
#define FUNCTION_COUNTER_HPP

#include <chrono>
#include <map>
#include <shared_mutex>
#include <source_location>
#include <string_view>
#include <vector>

#define COUNT_AND_TIME_CALL                                        \
    FunctionCounter::start_timing();                               \
    auto counter_guard = std::make_unique<int>(0);                 \
    (void)counter_guard;                                           \
    auto timer_end = [](void*) { FunctionCounter::end_timing(); }; \
    std::unique_ptr<void, decltype(timer_end)> timer_guard(        \
        counter_guard.get(), timer_end);

class FunctionCounter {
public:
    struct FunctionStats {
        size_t call_count = 0;
        std::chrono::nanoseconds total_time{0};
        std::chrono::nanoseconds min_time{std::chrono::nanoseconds::max()};
        std::chrono::nanoseconds max_time{std::chrono::nanoseconds::min()};
        std::vector<std::string_view> callers;
    };

    static void start_timing(const std::source_location location = std::source_location::current());
    static void end_timing();
    static void print_stats(size_t top_n = 0);
    static void reset_stats();
    static void save_stats(const std::string& filename);
    static void load_stats(const std::string& filename);
    static void set_performance_threshold(std::chrono::nanoseconds threshold);
    static void print_call_graph();

    template <typename Func>
    static void conditional_count(bool condition, Func&& func);

private:
    static inline std::map<std::string_view, FunctionStats> counts;
    static inline std::vector<std::pair<std::string_view, std::chrono::high_resolution_clock::time_point>> time_stack;
    static inline std::shared_mutex mutex;
    static inline std::chrono::nanoseconds performance_threshold;

    static void print_stats_header();
    static void print_function_stats(std::string_view func, const FunctionStats& stats);
    static std::string format_duration(std::chrono::nanoseconds ns);
};

template <typename Func>
void FunctionCounter::conditional_count(bool condition, Func&& func) {
    if (condition) {
        COUNT_AND_TIME_CALL;
        func();
    } else {
        func();
    }
}

#endif // FUNCTION_COUNTER_HPP
