#ifndef LITHIUM_BENCHMARK_HPP
#define LITHIUM_BENCHMARK_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <execution>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

class Benchmark {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    Benchmark(const std::string& name) : name_(name) {}

    template <typename Func>
    void Run(Func&& func, int iterations) {
        std::vector<Duration> durations(iterations);

        std::transform(std::execution::par, durations.begin(), durations.end(),
                       durations.begin(), [&func](const Duration&) {
                           TimePoint start = Clock::now();
                           func();
                           return Clock::now() - start;
                       });

        Duration totalDuration = std::accumulate(
            durations.begin(), durations.end(), Duration::zero());
        double averageDuration = static_cast<double>(totalDuration.count()) /
                                 iterations / 1000.0;  // in microseconds

        double variance =
            std::transform_reduce(
                std::execution::par, durations.begin(), durations.end(), 0.0,
                std::plus<>(),
                [&averageDuration](const Duration& d) {
                    double durationInMicroseconds =
                        static_cast<double>(d.count()) / 1000.0;
                    return std::pow(durationInMicroseconds - averageDuration,
                                    2);
                }) /
            iterations;

        double standardDeviation = std::sqrt(variance);

        results_.push_back({name_, totalDuration, averageDuration,
                            standardDeviation, iterations});
    }

    static void PrintResults() {
        std::cout << "Benchmark Results:\n";
        for (const auto& result : results_) {
            std::cout << std::setw(20) << std::left << result.name << ": "
                      << std::chrono::duration_cast<std::chrono::microseconds>(
                             result.totalDuration)
                             .count()
                      << " us (avg: " << std::setprecision(4)
                      << result.averageDuration
                      << " us, std dev: " << result.standardDeviation << " us, "
                      << result.iterations << " iterations)\n";
        }
    }

private:
    struct Result {
        std::string name;
        Duration totalDuration;
        double averageDuration;
        double standardDeviation;
        int iterations;
    };

    static inline std::vector<Result> results_;
    std::string name_;
};

#define BENCHMARK(name, func, iterations) Benchmark(name).Run(func, iterations)

std::vector<Benchmark::Result> Benchmark::results_;

#endif
