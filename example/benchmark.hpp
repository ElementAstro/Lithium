#include <chrono>
#include <cmath>
#include <functional>
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
        std::vector<Duration> durations;
        durations.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            TimePoint start = Clock::now();
            func();
            Duration elapsed = Clock::now() - start;
            durations.push_back(elapsed);
        }

        Duration totalDuration = std::accumulate(
            durations.begin(), durations.end(), Duration::zero());
        double averageDuration = static_cast<double>(totalDuration.count()) /
                                 iterations / 1000.0;  // in microseconds
        double variance = 0.0;
        for (const auto& d : durations) {
            variance += std::pow(
                static_cast<double>(d.count()) / 1000.0 - averageDuration, 2);
        }
        variance /= iterations;
        double standardDeviation = std::sqrt(variance);

        results_.push_back({name_, totalDuration, averageDuration,
                            standardDeviation, iterations});
    }

    static void PrintResults() {
        std::cout << "Benchmark Results:\n";
        for (const auto& result : results_) {
            std::cout << result.name << ": "
                      << std::chrono::duration_cast<std::chrono::microseconds>(
                             result.totalDuration)
                             .count()
                      << " us (avg: " << result.averageDuration
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