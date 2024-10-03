#include "benchmark.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>
#include <utility>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <intrin.h>
#include <psapi.h>
#if defined (__MINGW64__) || defined(__MINGW32__)
#include <x86intrin.h>
#endif
// clang-format on
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#include <unistd.h>
#endif

Benchmark::Benchmark(std::string suiteName, std::string name, Config config)
    : suiteName_(std::move(suiteName)),
      name_(std::move(name)),
      config_(std::move(config)) {}

void Benchmark::printResults(const std::string& suite) {
    std::scoped_lock lock(printMutex);
    std::cout << "Benchmark Results:\n";
    for (const auto& [suiteName, suiteResults] : results) {
        if (!suite.empty() && suite != suiteName)
            continue;
        std::cout << "Suite: " << suiteName << "\n";
        for (const auto& result : suiteResults) {
            std::cout << std::format(
                "{:<20}: Avg: {:.4f} us, Min: {:.4f} us, Max: {:.4f} us, "
                "Median: {:.4f} us, "
                "Std Dev: {:.4f} us, Iters: {:>4}, Throughput: {:.2f} "
                "ops/sec, "
                "Avg Mem: {:.2f} MB, Peak Mem: {:.2f} MB\n",
                result.name, result.averageDuration, result.minDuration,
                result.maxDuration, result.medianDuration,
                result.standardDeviation, result.iterations, result.throughput,
                result.avgMemoryUsage / (1024.0 * 1024.0),
                result.peakMemoryUsage / (1024.0 * 1024.0));
        }
    }
}

void Benchmark::exportResults(const std::string& filename) {
    Log("Exporting results to file: " + filename);
    std::scoped_lock lock(printMutex);
    nlohmann::json jsonResults;

    for (const auto& [suiteName, suiteResults] : results) {
        nlohmann::json suiteJson;
        for (const auto& result : suiteResults) {
            nlohmann::json resultJson = {
                {"name", result.name},
                {"averageDuration", result.averageDuration},
                {"minDuration", result.minDuration},
                {"maxDuration", result.maxDuration},
                {"medianDuration", result.medianDuration},
                {"standardDeviation", result.standardDeviation},
                {"iterations", result.iterations},
                {"throughput", result.throughput},
                {"avgMemoryUsage", result.avgMemoryUsage},
                {"peakMemoryUsage", result.peakMemoryUsage}};
            suiteJson.push_back(resultJson);
        }
        jsonResults[suiteName] = suiteJson;
    }

    std::filesystem::path filePath(filename);
    if (filePath.extension() == ".json") {
        std::ofstream file(filename);
        file << jsonResults.dump(4);
    } else if (filePath.extension() == ".csv") {
        std::ofstream file(filename);
        file << "Suite,Name,AvgDuration,MinDuration,MaxDuration,"
                "MedianDuration,StdDev,Iterations,Throughput,AvgMemory,"
                "PeakMemory\n";
        for (const auto& [suiteName, suiteResults] : results) {
            for (const auto& result : suiteResults) {
                file << std::format("{},{},{},{},{},{},{},{},{},{},{}\n",
                                    suiteName, result.name,
                                    result.averageDuration, result.minDuration,
                                    result.maxDuration, result.medianDuration,
                                    result.standardDeviation, result.iterations,
                                    result.throughput, result.avgMemoryUsage,
                                    result.peakMemoryUsage);
            }
        }
    }
    Log("Completed exporting results to file: " + filename);
}

void Benchmark::warmupRun(const auto& setupFunc, const auto& func,
                          const auto& teardownFunc) {
    auto setupData = setupFunc();
    func(setupData);  // Warmup operation
    teardownFunc(setupData);
}

auto Benchmark::totalDuration(const std::vector<Duration>& durations)
    -> Duration {
    return std::accumulate(durations.begin(), durations.end(),
                           Duration::zero());
}

void Benchmark::analyzeResults(const std::vector<Duration>& durations,
                               const std::vector<MemoryStats>& memoryStats,
                               const std::vector<CPUStats>& cpuStats,
                               std::size_t totalOpCount) {
    std::vector<double> microseconds;
    std::ranges::transform(
        durations.begin(), durations.end(), std::back_inserter(microseconds),
        [](const Duration& duration) {
            return std::chrono::duration<double, std::micro>(duration).count();
        });

    std::ranges::sort(microseconds.begin(), microseconds.end());
    double totalDuration =
        std::accumulate(microseconds.begin(), microseconds.end(), 0.0);
    auto iterations = static_cast<int>(microseconds.size());

    Result result;
    result.name = name_;
    result.averageDuration = totalDuration / iterations;
    result.minDuration = microseconds.front();
    result.maxDuration = microseconds.back();
    result.medianDuration = microseconds[iterations / 2];
    result.standardDeviation = calculateStandardDeviation(microseconds);
    result.iterations = iterations;
    result.throughput =
        static_cast<double>(totalOpCount) / (totalDuration * 1e-6);

    result.avgMemoryUsage =
        std::accumulate(memoryStats.begin(), memoryStats.end(), 0.0,
                        [](double sum, const MemoryStats& stats) {
                            return sum +
                                   static_cast<double>(stats.currentUsage);
                        }) /
        iterations;
    result.peakMemoryUsage =
        std::ranges::max_element(
            memoryStats.begin(), memoryStats.end(),
            [](const MemoryStats& a, const MemoryStats& b) {
                return a.peakUsage < b.peakUsage;
            })
            ->peakUsage;

    result.avgCPUStats = calculateAverageCpuStats(cpuStats);

    results[suiteName_].push_back(result);
}

auto Benchmark::calculateStandardDeviation(const std::vector<double>& values)
    -> double {
    double mean = std::accumulate(values.begin(), values.end(), 0.0) /
                  static_cast<double>(values.size());
    double variance =
        std::accumulate(values.begin(), values.end(), 0.0,
                        [mean](double sum, double value) {
                            return sum + std::pow(value - mean, 2);
                        }) /
        static_cast<double>(values.size());
    return std::sqrt(variance);
}

auto Benchmark::calculateAverageCpuStats(const std::vector<CPUStats>& stats)
    -> CPUStats {
    CPUStats avg{};
    for (const auto& stat : stats) {
        avg.instructionsExecuted += stat.instructionsExecuted;
        avg.cyclesElapsed += stat.cyclesElapsed;
        avg.branchMispredictions += stat.branchMispredictions;
        avg.cacheMisses += stat.cacheMisses;
    }
    size_t count = stats.size();
    avg.instructionsExecuted /= static_cast<long long>(count);
    avg.cyclesElapsed /= static_cast<long long>(count);
    avg.branchMispredictions /= static_cast<long long>(count);
    avg.cacheMisses /= static_cast<long long>(count);
    return avg;
}

auto Benchmark::subtractCpuStats(const CPUStats& end,
                                 const CPUStats& start) -> CPUStats {
    return {end.instructionsExecuted - start.instructionsExecuted,
            end.cyclesElapsed - start.cyclesElapsed,
            end.branchMispredictions - start.branchMispredictions,
            end.cacheMisses - start.cacheMisses};
}

auto Benchmark::getMemoryUsage() -> MemoryStats {
    MemoryStats stats{};
#ifdef _WIN32
    if (PROCESS_MEMORY_COUNTERS_EX pmc; GetProcessMemoryInfo(
            GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        stats.currentUsage = pmc.WorkingSetSize;
        stats.peakUsage = pmc.PeakWorkingSetSize;
    }
#elif defined(__unix__) || defined(__APPLE__)
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    stats.currentUsage = rusage.ru_maxrss * 1024;  // Convert from KB to bytes
    stats.peakUsage = stats.currentUsage;  // Peak usage not directly available,
                                           // using current as approximation
#endif
    return stats;
}

auto Benchmark::getCpuStats() -> CPUStats {
    CPUStats stats = {0, 0, 0, 0};

#ifdef _WIN32
    // Windows implementation
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);
    if (cpuInfo[0] >= 0x0A) {
        unsigned long long instructionsRetired;
        unsigned long long unhaltedCoreCycles;
        unsigned long long branchMisses;
        unsigned long long cacheMisses;
#ifdef _MSC_VER
        instructionsRetired = __readpmc(0);
        unhaltedCoreCycles = __readpmc(1);
        branchMisses = __readpmc(3);
        cacheMisses = __readpmc(5);
#else
        // TODO: Implement for MinGW
        // instructionsRetired = __builtin_ia32_rdpmc(0);
        // unhaltedCoreCycles = __builtin_ia32_rdpmc(1);
        // branchMisses = __builtin_ia32_rdpmc(3);
        // cacheMisses = __builtin_ia32_rdpmc(5);
#endif
        stats.instructionsExecuted = instructionsRetired;
        stats.cyclesElapsed = unhaltedCoreCycles;
        stats.branchMispredictions = branchMisses;
        stats.cacheMisses = cacheMisses;
    }

#elif defined(__linux__)
    // Linux implementation using perf_event
    struct perf_event_attr pe;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (fd != -1) {
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        read(fd, &stats.instructionsExecuted, sizeof(int64_t));
        close(fd);
    }

    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (fd != -1) {
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        read(fd, &stats.cyclesElapsed, sizeof(int64_t));
        close(fd);
    }

    pe.config = PERF_COUNT_HW_BRANCH_MISSES;
    fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (fd != -1) {
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        read(fd, &stats.branchMispredictions, sizeof(int64_t));
        close(fd);
    }

    pe.config = PERF_COUNT_HW_CACHE_MISSES;
    fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (fd != -1) {
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        read(fd, &stats.cacheMisses, sizeof(int64_t));
        close(fd);
    }

#elif defined(__APPLE__)
    // macOS implementation
    // Note: macOS doesn't provide as detailed CPU stats as Linux or Windows
    // We'll use what's available
    uint64_t cpuCycles = 0;
    size_t size = sizeof(cpuCycles);
    sysctlbyname("hw.cpu_cycles", &cpuCycles, &size, NULL, 0);
    stats.cyclesElapsed = cpuCycles;

    // Other stats are not readily available on macOS without kernel
    // extensions For a production system, you might want to use third-party
    // libraries or tools

#endif

    return stats;
}

void Benchmark::Log(const std::string& message) {
    std::scoped_lock lock(logMutex);
    std::cout << "[LOG] " << message << std::endl;
}

std::map<std::string, std::vector<Benchmark::Result>> Benchmark::results;
std::mutex Benchmark::printMutex;
std::mutex Benchmark::logMutex;